#include "http/controller/group_controller.h"
#include "model/group.h"
#include "http/helper/helper.h"

#include <stdio.h>
#include <string.h>

/**
 * It creates a new group
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *create_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  char *name = request->query.search(&request->query, "name", sizeof(char[strlen("name")]));
  char *description = request->query.search(&request->query, "description", sizeof(char[strlen("description")]));
  char *avatar = request->query.search(&request->query, "avatar", sizeof(char[strlen("avatar")]));

  if (name == NULL)
  {
    return format_422();
  }

  if (get_group_by_name(name) != NULL)
  {
    return format_422();
  }

  struct Group *group = group_new(name, description, avatar, user->id);
  if (group->save(group))
  {
    return format_500();
  }

  char *response = group->to_json(group);
  group_free(group);

  return format_200_with_content_type(response, "application/json");
}

char *update_group_(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  char *code = request->query.search(&request->query, "code", sizeof(char[strlen("code")]));
  char *name = request->query.search(&request->query, "name", sizeof(char[strlen("name")]));
  char *description = request->query.search(&request->query, "description", sizeof(char[strlen("description")]));
  char *avatar = request->query.search(&request->query, "avatar", sizeof(char[strlen("avatar")]));

  if (code == NULL)
  {
    return format_422();
  }

  struct Group *group = get_group_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (group->owner_id != user->id)
  {
    return format_403();
  }

  if (name != NULL)
    group->name = strdup(name);

  if (description != NULL)
  {
    group->description = strdup(description);
  }

  if (avatar != NULL)
  {
    group->avatar = strdup(avatar);
  }

  if (group->update(group))
  {
    return format_500();
  }

  char *response = group->to_json(group);
  group_free(group);

  return format_200_with_content_type(response, "application/json");
}
