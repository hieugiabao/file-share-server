
#ifndef GROUP_CONTROLLER_H
#define GROUP_CONTROLLER_H

#include "networking/http/http_server.h"

char *create_group(struct HTTPServer *server, struct HTTPRequest *request);
char *update_group_(struct HTTPServer *server, struct HTTPRequest *request);
char *delete_group_(struct HTTPServer *server, struct HTTPRequest *request);

#endif
