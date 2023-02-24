
#ifndef GROUP_CONTROLLER_H
#define GROUP_CONTROLLER_H

#include "networking/http/http_server.h"

char *create_group(struct HTTPServer *server, struct HTTPRequest *request);
char *update_group(struct HTTPServer *server, struct HTTPRequest *request);
char *get_group_members(struct HTTPServer *server, struct HTTPRequest *request);
char *get_my_groups(struct HTTPServer *server, struct HTTPRequest *request);
char *delete_group(struct HTTPServer *server, struct HTTPRequest *request);
char *join_group(struct HTTPServer *server, struct HTTPRequest *request);
char *leave_group(struct HTTPServer *server, struct HTTPRequest *request);
char *kick_member_group(struct HTTPServer *server, struct HTTPRequest *request);

#endif
