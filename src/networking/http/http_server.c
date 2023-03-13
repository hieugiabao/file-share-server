#include "networking/http/http_server.h"
#include "systems.h"
#include "logger/logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Public member methods prototypes */

void http_launch(struct HTTPServer *server);
void *http_handler(void *arg);

void register_routes(struct HTTPServer *server, char *(*callback)(struct HTTPServer *server, struct HTTPRequest *request), char *uri, int num_methods, ...);

/* Public helper functions */

char *_404(size_t *size);
char *_400(size_t *size);
char *_455(size_t *size);
const char *get_content_type(const char *uri);
char *server_resource(char *uri, size_t *size);

int is_match_method(char *method, int methods[9]);

/* Private data types */

/**
 * The client server struct is used as an argument for the handler method.
 * By necessity, handler takes one void pointer as an argument, but both the client address and server reference are required.
 */
struct ClientServer
{
  int client;                // The client socket
  struct HTTPServer *server; // The server instance
};

/**
 * The route struct is stored in the HTTPServer.routes dictionary as an encapsulation of the methods, uri, and function associated with a given route.
 */
struct Route
{
  int methods[9]; // The HTTP methods that are allowed for this route
  char *uri;      // The URI that this route is for
  // The callback function that will be called when this route is requested
  char *(*route_callback)(struct HTTPServer *server, struct HTTPRequest *request);
};

/* Constructor */

/**
 * It creates a server object, initializes it, and returns it
 *
 * @param interface The IP address of the interface you want to listen on.
 * @param port The port to listen on.
 *
 * @return A struct HTTPServer
 */
struct HTTPServer http_server_constructor(u_long interface, int port)
{
  struct HTTPServer server;
  server.server = server_constructor(AF_INET, SOCK_STREAM, 0, interface, port, 255);
  server.routes = dictionary_constructor(compare_string_keys);
  server.register_routes = register_routes;
  server.launch = http_launch;
  server.pool = NULL;
  log_info("Http server initialized on port %s:%ld", inet_ntoa(server.server.address.sin_addr), ntohs(server.server.address.sin_port));

  return server;
}

/**
 * It's a wrapper around the server destructor that also destroys the routes dictionary
 *
 * @param server The server object.
 */
void http_server_destructor(struct HTTPServer *server)
{
  if (server->pool != NULL)
    thread_pool_destructor(server->pool);
  server_destructor(&server->server);
  dictionary_destructor(&server->routes, NULL, NULL);
}

/**
 * Adds a specified route to the dictionary associated with a given HTTP server.
 *
 * @param server The server to register the route with.
 * @param callback The function to be called when the route is accessed.
 * @param uri The URI to register.
 * @param num_methods The number of methods that the route should be registered for.
 */
void register_routes(struct HTTPServer *server, char *(*callback)(struct HTTPServer *server, struct HTTPRequest *request), char *uri, int num_methods, ...)
{
  struct Route route;
  // Iterate over the list of methods provided.
  va_list methods;
  va_start(methods, num_methods);
  for (int i = 0; i < num_methods; i++)
  {
    route.methods[i] = va_arg(methods, int);
  }
  // Register the URI.
  char buffer[strlen(uri)];
  route.uri = buffer;
  strcpy(route.uri, uri);
  // Copy the desired function.
  route.route_callback = callback;
  // Save the route in the server's dictionary.
  server->routes.insert(&server->routes, uri, sizeof(char[strlen(uri)]), &route, sizeof(route));
}

/**
 * It creates a thread pool, accepts a client, and passes the client off to the thread pool
 *
 * @param server A pointer to the HTTPServer struct.
 */
void http_launch(struct HTTPServer *server)
{
  log_info("Http server launched... Waiting for clients...");
  // Initialize a thread pool to handle clients.
  struct ThreadPool *thread_pool = thread_pool_constructor(20);
  server->pool = thread_pool;
  // Cast some of the member variables from the server.
  struct sockaddr *sock_addr = (struct sockaddr *)&server->server.address;
  socklen_t address_length = (socklen_t)sizeof(server->server.address);
  // An infinite loop allows the server to continuously accept new clients.
  while (1)
  {
    // Create an instance of the ClientServer struct.
    struct ClientServer *client_server = malloc(sizeof(struct ClientServer));
    // Accept an incoming connection.
    client_server->client = accept(server->server.socket, sock_addr, &address_length);
    client_server->server = server;
    // Pass the client off to the thread pool.
    struct ThreadJob job = thread_job_constructor(http_handler, client_server);
    thread_pool->add_work(thread_pool, job);
  }
}

/**
 * The handler is used in a multithreaded environment to handle incoming connections.
 *
 * @param arg A pointer to a ClientServer struct.
 *
 * @return A pointer to a void.
 */
void *http_handler(void *arg)
{
  // Cast the argument back to a ClientServer struct.
  struct ClientServer *client_server = (struct ClientServer *)arg;
  // Read the client's request.
  char request_string[60000];
  // int bytes_received = 0;
  // while (recv(client_server->client, request_string, 60000, 0) > 0)
  //   bytes_received += strlen(request_string);
  ssize_t byte_received = recv(client_server->client, request_string, 60000, 0);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 50000;
  setsockopt(client_server->client, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
  while ((byte_received = recv(client_server->client, request_string+byte_received, 60000, 0)) > 0)
  {
    log_trace("Received more %ld bytes", byte_received);
  }
  log_trace("Request: %s", request_string);
  // Parse the request string into a usable format.
  struct HTTPRequest request = http_request_constructor(request_string);
  // Extract the URI from the request.
  char *uri = request.request_line.search(&request.request_line, "uri", sizeof("uri"));
  char *method = request.request_line.search(&request.request_line, "method", sizeof("method"));
  // Find the corresponding route in the server's dictionary.
  // log
  log_info(
      "Request from %s:%ld - method: %s - route: %s",
      inet_ntoa(client_server->server->server.address.sin_addr),
      ntohs(client_server->server->server.address.sin_port), method, uri);

  struct Route *route = client_server->server->routes.search(&client_server->server->routes, uri, sizeof(char[strlen(uri)]));
  // Process the request and respond to the client.
  char *response;
  size_t response_size;

  if (route)
  {
    if (is_match_method(method, route->methods))
    {
      response = route->route_callback(client_server->server, &request);
      response_size = sizeof(char[strlen(response)]);
    }
    else
    {
      response = _455(&response_size);
    }
  }
  else
  {
    response = server_resource(uri, &response_size);
  }
  write(client_server->client, response, response_size);
  write(client_server->client, response, sizeof(char[strlen(response)]));
  close(client_server->client);
  // Free the ClientServer object.
  free(client_server);

  http_request_destructor(&request);

  return NULL;
}

/**
 * Joins the contents of multiple files into one.
 *
 * @param num_templates The number of templates to render.
 *
 * @return A pointer to the first character in the buffer.
 */
char *render_template(int num_templates, ...)
{
  // Create a buffer to store the data in.
  char *buffer = malloc(30000);
  int buffer_position = 0;
  char c;
  FILE *file;
  // Iterate over the files given as arguments.
  va_list files;
  va_start(files, num_templates);
  // Read the contents of each file into the buffer.
  for (int i = 0; i < num_templates; i++)
  {
    char *path = va_arg(files, char *);
    file = fopen(path, "r");
    if (file == NULL)
      continue;
    while ((c = fgetc(file)) != EOF)
    {
      buffer[buffer_position] = c;
      buffer_position += 1;
    }
  }
  va_end(files);
  return buffer;
}

/**
 * It takes a pointer to a size_t, and returns a pointer to a char
 *
 * @param size the pointer to the size of the response
 *
 * @return A pointer to a string that is the 404 response.
 */
char *_404(size_t *size)
{
  const char *c404 = "HTTP/1.1 404 Not Found\r\n"
                     "Connection: close\r\n"
                     "Content-Length: ";
  char *template_response = render_template(1, "public/404.html");
  char *response = malloc(strlen(template_response) + strlen(c404) + 10);
  sprintf(response, "%s%ld\r\n\r\n%s", c404, strlen(template_response), template_response);
  *size = strlen(response);
  return response;
}

/**
 * It takes a pointer to a size_t, and returns a pointer to a string
 *
 * @param size the pointer to the size of the response
 *
 * @return A pointer to a string that is the response to a 400 error.
 */
char *_400(size_t *size)
{
  const char *c400 = "HTTP/1.1 400 Bad Request\r\n"
                     "Connection: close\r\n"
                     "Content-Length: ";
  char *template_response = render_template(1, "public/400.html");
  char *response = malloc(strlen(template_response) + strlen(c400) + 10);
  sprintf(response, "%s%ld\r\n\r\n%s", c400, strlen(template_response), template_response);
  *size = strlen(response);
  return response;
}

char *_455(size_t *size)
{
  const char *c455 = "HTTP/1.1 455 Method Not Allowed\r\n"
                     "Connection: close\r\n"
                     "Content-Length: 0\r\n\r\n";

  char *response = malloc(strlen(c455) + 10);
  sprintf(response, "%s", c455);
  *size = strlen(response);
  return response;
}

/**
 * It returns the content type of a file based on its extension
 *
 * @param uri The URI of the file to be served.
 *
 * @return The content type of the file.
 */
const char *get_content_type(const char *uri)
{
  const char *last_dot = strrchr(uri, '.');
  if (last_dot)
  {
    if (strcmp(last_dot, ".css") == 0)
      return "text/css";
    if (strcmp(last_dot, ".csv") == 0)
      return "text/csv";
    if (strcmp(last_dot, ".gif") == 0)
      return "image/gif";
    if (strcmp(last_dot, ".htm") == 0)
      return "text/html";
    if (strcmp(last_dot, ".html") == 0)
      return "text/html";
    if (strcmp(last_dot, ".ico") == 0)
      return "image/x-icon";
    if (strcmp(last_dot, ".jpeg") == 0)
      return "image/jpeg";
    if (strcmp(last_dot, ".jpg") == 0)
      return "image/jpeg";
    if (strcmp(last_dot, ".js") == 0)
      return "application/javascript";
    if (strcmp(last_dot, ".json") == 0)
      return "application/json";
    if (strcmp(last_dot, ".png") == 0)
      return "image/png";
    if (strcmp(last_dot, ".pdf") == 0)
      return "application/pdf";
    if (strcmp(last_dot, ".svg") == 0)
      return "image/svg+xml";
    if (strcmp(last_dot, ".txt") == 0)
      return "text/plain";
  }

  return "application/octet-stream";
}

/**
 * It reads the file from the disk and returns it as a string
 *
 * @param uri The URI of the request.
 * @param size The size of the response.
 *
 * @return A pointer to a buffer containing the HTTP response.
 */
char *server_resource(char *uri, size_t *size)
{
  if (strcmp(uri, "/") == 0)
    uri = "/index.html";

  if (strlen(uri) > 100)
    return _400(size);

  if (strstr(uri, ".."))
    return _404(size);

  char full_path[128];
  sprintf(full_path, "public%s", uri);

  FILE *fp = fopen(full_path, "rb");

  if (!fp)
  {
    return _404(size);
  }

  size_t file_size = get_file_size(full_path);

  if (file_size > 1048500) // 1MB
    return _400(size);

  const char *content_type = get_content_type(full_path);
#define BSIZE 1048576 // 1MB
#define USIZE 1024
  char *buffer = malloc(BSIZE);
  sprintf(buffer, "HTTP/1.1 200 OK\r\n");
  sprintf(buffer + strlen(buffer), "Connection: close\r\n");
  sprintf(buffer + strlen(buffer), "Content-Length: %ld\r\n", file_size);
  sprintf(buffer + strlen(buffer), "Content-Type: %s\r\n\r\n", content_type);

  *size = strlen(buffer);

  char temp[USIZE];
  int r = fread(temp, 1, USIZE, fp);
  while (r)
  {
    memcpy(buffer + *size, temp, r);
    *size += r;
    r = fread(temp, 1, USIZE, fp);
  }
  fclose(fp);
  return buffer;
}

/**
 * It takes a string and an array of integers, and returns 1 if the string matches one of the integers in the array
 *
 * @param method The HTTP method to match.
 * @param methods an array of ints that represent the HTTP methods that are allowed for this route.
 *
 * @return The return value is the code of the method.
 */
int is_match_method(char *method, int methods[9])
{
  int code;
  if (strcmp(method, "GET") == 0)
    code = GET;
  if (strcmp(method, "POST") == 0)
    code = POST;
  if (strcmp(method, "PUT") == 0)
    code = PUT;
  if (strcmp(method, "DELETE") == 0)
    code = DELETE;
  if (strcmp(method, "HEAD") == 0)
    code = HEAD;
  if (strcmp(method, "OPTIONS") == 0)
    code = OPTIONS;
  if (strcmp(method, "CONNECT") == 0)
    code = CONNECT;
  if (strcmp(method, "TRACE") == 0)
    code = TRACE;
  if (strcmp(method, "PATCH") == 0)
    code = PATCH;

  for (int i = 0; i < 9; i++)
  {
    if (methods[i] == code)
      return 1;
  }

  return 0;
}
