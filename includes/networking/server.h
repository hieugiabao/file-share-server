
#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "data_structures/dictionary.h"

/* The Server struct is used as the basis for nodes that need to operate as servers.
 Generically, the server connects to the network and listens on a given port. */
struct Server
{
  /* Public variables */

  int domain;                 // AF_INET, AF_INET6, AF_UNIX, AF_LOCAL, AF_ROUTE, AF_KEY
  int service;                // SOCK_STREAM, SOCK_DGRAM, SOCK_SEQPACKET, SOCK_RAW
  int protocol;               // IPPROTO_TCP, IPPROTO_UDP, IPPROTO_SCTP, IPPROTO_TIPC
  u_long interface;           // INADDR_ANY, INADDR_LOOPBACK, INADDR_BROADCAST, INADDR_NONE
  int port;                   // 0 - 65535
  int backlog;                // 0 - 255
  struct sockaddr_in address; // IPv4 address
  int socket;                 // Socket file descriptor
  struct Dictionary routes;   // Dictionary of routes

  /* Public methods */

  void (*register_routes)(struct Server *server, char *(*route_function)(void *arg), char *path);
};

struct ServerRoute
{
  char *(*route_function)(void *arg); // Function to call when route is hit
};

/* Creates a new server struct and returns a pointer to it. */
struct Server server_constructor(int domain, int service, int protocol, u_long interface, int port, int backlog);

/* Frees the memory allocated to a server struct. */
void server_destructor(struct Server *server);

#endif /* SERVER_H */
