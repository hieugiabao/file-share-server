
#ifndef USER_CONTROLLER_H
#define USER_CONTROLLER_H

#include "networking/http/http_server.h"

char *login(struct HTTPServer *server, struct HTTPRequest *request);
char *logout(struct HTTPServer *server, struct HTTPRequest *request);
char *register_user(struct HTTPServer *server, struct HTTPRequest *request);
char *get_me_info(struct HTTPServer *server, struct HTTPRequest *request);
char *get_user_info(struct HTTPServer *server, struct HTTPRequest *request);

#endif
