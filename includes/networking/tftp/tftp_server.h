#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include "networking/server.h"

struct TFTPServer
{
  /* Public variables */

  struct Server server; // A generic server object to connect to the network with the appropriate protocols.
  char is_allow_upload; // Whether or not the server allows clients to upload files.
  char upload_dir[256]; // The directory to upload files to.

  /* Public methods */

  // The launch method starts the server and listens for connections.
  void (*launch)(struct TFTPServer *server);
};

struct TFTPServer tftp_server_constructor(u_long interface, int port, char is_allow_upload, char *upload_dir);
void tftp_server_destructor(struct TFTPServer *server);

#endif
