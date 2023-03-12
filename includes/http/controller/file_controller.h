
#ifndef FILE_CONTROLLER_H
#define FILE_CONTROLLER_H

#include "networking/http/http_server.h"

char *create_file(struct HTTPServer *server, struct HTTPRequest *request);
char *save_file(struct HTTPServer *server, struct HTTPRequest *request);
char *update_file(struct HTTPServer *server, struct HTTPRequest *request);
char *delete_file(struct HTTPServer *server, struct HTTPRequest *request);
char *get_file(struct HTTPServer *server, struct HTTPRequest *request);

#endif // FILE_CONTROLLER_H