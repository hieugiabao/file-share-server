
#ifndef DIRECTORY_CONTROLLER_H
#define DIRECTORY_CONTROLLER_H

#include "networking/http/http_server.h"

char *make_directory(struct HTTPServer *server, struct HTTPRequest *request);
char *update_directory(struct HTTPServer *server, struct HTTPRequest *request);
char *delete_directory(struct HTTPServer *server, struct HTTPRequest *request);
char *get_directory_children(struct HTTPServer *server, struct HTTPRequest *request);
char *get_group_node_tree(struct HTTPServer *server, struct HTTPRequest *request);

#endif // DIRECTORY_CONTROLLER_H