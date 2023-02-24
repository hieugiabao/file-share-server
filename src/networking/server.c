#include "networking/server.h"
#include "data_structures/dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void register_routes_server(struct Server *server, char *(*route_function)(void *arg), char *path);

/* Server constructor */

/**
 * It creates a socket, binds it to the network, and starts listening for connections
 *
 * @param domain The domain of the socket.
 * @param service The type of socket to create.
 * @param protocol The protocol to use.
 * @param interface The IP address of the interface you want to listen on.
 * @param port The port number to listen on.
 * @param backlog The maximum number of connections that can be queued for this socket.
 *
 * @return A server object.
 */
struct Server server_constructor(int domain, int service, int protocol, u_long interface, int port, int backlog)
{
  struct Server server;

  // Define the basic parameters of the server.
  server.domain = domain;
  server.service = service;
  server.protocol = protocol;
  server.interface = interface;
  server.port = port;
  server.backlog = backlog;

  // Use the aforementioned parameters to construct the server's address.
  server.address.sin_family = domain;
  server.address.sin_port = htons(port);
  server.address.sin_addr.s_addr = htonl(interface);

  // Create a socket for the server
  server.socket = socket(domain, service, protocol);

  // Init route dictionary
  server.routes = dictionary_constructor(compare_string_keys);

  server.register_routes = register_routes_server;

  // Confirm the connection was successful.
  if (server.socket <= 0)
  {
    printf("Failed to create socket...\n");
    exit(1);
  }
  // Attempt to bind the socket to the network.
  if (bind(server.socket, (struct sockaddr *)&server.address, sizeof(server.address)) < 0)
  {
    printf("Failed to bind socket...\n");
    exit(1);
  }
  // Start to listen on the network.
  if (service == SOCK_STREAM && listen(server.socket, server.backlog) < 0)
  {
    printf("Failed to listen on socket...\n");
    exit(1);
  }
  return server;
}

/**
 * It creates a socket, binds it to a port, and listens for connections
 *
 * @param server a pointer to the server object
 */
void server_destructor(struct Server *server)
{
  close(server->socket);
  dictionary_destructor(&server->routes, NULL, NULL);
}

/**
 * It takes a pointer to a function that returns a char pointer and takes a void pointer as an argument, and a string that
 * represents the path to the route. It then creates a struct ServerRoute, which is a struct that contains a pointer to a
 * function that returns a char pointer and takes a void pointer as an argument, and inserts it into the server's routes
 * map
 *
 * @param server The server to register the route to.
 * @param route_function A function that returns a string.
 * @param path The path to the route.
 */
void register_routes_server(struct Server *server, char *(*route_function)(void *arg), char *path)
{
  struct ServerRoute route;
  route.route_function = route_function;
  server->routes.insert(&server->routes, path, sizeof(char[strlen(path)]), &route, sizeof(route));
}
