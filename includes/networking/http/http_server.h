

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "networking/server.h"
#include "http_request.h"

/**
 * The HTTPServer struct is the basis for servers intended to read and recieve HTTP protocols.
 * To utilize the server, instantiate an HTTPServer object with the constructor, register routes with the member method, and launch the server.
 */
struct HTTPServer
{
  /* Public member variables */

  struct Server server;     // A generic server object to connect to the network with the appropriate protocols.
  struct Dictionary routes; // A dictionary of routes registered on the server with URL's as keys.

  /* Public member methods */

  // This method is used to register URL's as routes to the server.
  void (*register_routes)(struct HTTPServer *server, char *(*route_function)(struct HTTPServer *server, struct HTTPRequest *request), char *uri, int num_methods, ...);
  // The launch sequence begins an infinite loop where the server listens for and handles incoming connections.
  void (*launch)(struct HTTPServer *server);
};

/**
 * The HTTPMethods enum lists the various HTTP methods for easy referral.
 */
enum HTTPMethods
{
  GET,
  POST,
  PUT,
  DELETE,
  HEAD,
  OPTIONS,
  TRACE,
  CONNECT,
  PATCH
};

/* Constructor and destructor */

struct HTTPServer http_server_constructor(u_long interface, int port);

/* Public helper functions */

char *render_template(int num_templates, ...);

#endif // HTTP_SERVER_H
