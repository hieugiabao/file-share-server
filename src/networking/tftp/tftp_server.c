
#include "networking/tftp/tftp_server.h"
#include "networking/tftp/header.h"
#include "networking/tftp/tftp_client_handle.h"
#include "systems/thread_pool.h"

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

void tftp_launch(struct TFTPServer *server);
void *tftp_handler(void *arg);

/* The client server struct is used as an argument for the handler method. */
struct ClientServer
{
  uint8_t *data;                     // The data received from the client
  size_t bytes_received;             // The number of bytes received from the client
  struct sockaddr_in client_address; // The address of the client
  struct TFTPServer *server;         // The server instance
};

/* Constructor */

/**
 * It creates a new TFTPServer object, sets its properties, and returns it
 *
 * @param interface The IP address of the interface you want to bind to. maybe INADDR_ANY, INADDR_BROADCAST, or INADDR_LOOPBACK, or something else.
 * @param port The port to listen on.
 * @param is_allow_upload If this is set to 1, then the server will allow uploads. If it's set to 0, then it won't.
 * @param upload_dir The directory where the uploaded files will be stored.
 *
 * @return A struct TFTPServer
 */
struct TFTPServer
tftp_server_constructor(u_long interface, int port, char is_allow_upload, char *upload_dir)
{
  struct TFTPServer tftp_server;
  tftp_server.server = server_constructor(AF_INET, SOCK_DGRAM, 0, interface, port, 0);
  tftp_server.is_allow_upload = is_allow_upload;
  strcpy(tftp_server.upload_dir, upload_dir);
  tftp_server.launch = tftp_launch;

  return tftp_server;
}

/**
 * It creates a thread pool, then waits for connections and adds a job to the thread pool for each connection
 *
 * @param server The server instance.
 *
 * @return A pointer to the newly created TFTPServer struct.
 */
void tftp_launch(struct TFTPServer *server)
{
  if (server->server.socket <= 0)
  {
    log_fatal("Server is not initialized");
    return;
  }
  // Initialize a thread pool to handle clients.
  struct ThreadPool *thread_pool = thread_pool_constructor(10);
  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);
  uint8_t data[BUF_SIZE];

  log_info("Server is initialized at %s:%d", inet_ntoa(server->server.address.sin_addr), ntohs(server->server.address.sin_port));
  log_info("Waiting for connections...");
  while (1)
  {
    size_t bytes_received = recvfrom(server->server.socket, data, BUF_SIZE, 0,
                                     (struct sockaddr *)&client_address, &client_len);

    if (bytes_received < 1)
    {
      log_error("connection closed by client");
      continue;
    }
    // Create an instance of the ClientServer struct.
    struct ClientServer *client_server = malloc(sizeof(struct ClientServer));
    client_server->data = data;
    client_server->bytes_received = bytes_received;
    client_server->client_address = client_address;
    client_server->server = server;

    struct ThreadJob job = thread_job_constructor(tftp_handler, client_server);
    thread_pool->add_work(thread_pool, job);
  }
}

/**
 * It's a destructor for the TFTPServer struct
 *
 * @param server The server object.
 */
void tftp_server_destructor(struct TFTPServer *server)
{
  server_destructor(&server->server);
}

/**
 * It creates a handler for the client, handles the client, and then frees the handler
 *
 * @param arg A pointer to a struct ClientServer.
 *
 * @return The return value is a pointer to the thread.
 */
void *tftp_handler(void *arg)
{
  log_info("Start handler");
  struct ClientServer *client_server = (struct ClientServer *)arg;
  TFTPClientHandler *handler = create_handler(client_server->data, client_server->bytes_received, &client_server->client_address, client_server->server);
  if (handler == NULL)
  {
    log_error("Failed to create handler");
  }
  else
  {
    if (handle_client(handler) == 0)
      log_info("Closing connection with %s:%d! Done request", inet_ntoa(client_server->client_address.sin_addr), ntohs(client_server->client_address.sin_port));
    free_handler(handler);
  }
  return NULL;
}
