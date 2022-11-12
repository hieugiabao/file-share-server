#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include "tftp_client_handle.h"

extern int sock_fd;
extern int is_upload_allowed;
extern char *upload_dir;
extern char *server_host;

int init_server(const char* host, const char *port, const char *root_dir, int allow_upload);
int close_server();
void serve(void);

#endif