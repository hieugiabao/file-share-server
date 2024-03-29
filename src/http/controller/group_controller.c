#include "http/controller/group_controller.h"
#include "model/group.h"
#include "http/helper/helper.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

  char *name = request->body.search(&request->body, "name", sizeof(char[strlen("name")]));
  char *description = request->body.search(&request->body, "description", sizeof(char[strlen("description")]));
  char *avatar = request->body.search(&request->body, "avatar", sizeof(char[strlen("avatar")]));

  if (name == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_new(name, description, avatar, user->id);
  if (group->save(group))
  {
    return format_500();
  }

  char *response = group->to_json(group);
  group_free(group);
  user_free(user);

  return format_200_with_content_type(response, "application/json");
}

char *update_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->body.search(&request->body, "code", sizeof(char[strlen("code")]));
  char *name = request->body.search(&request->body, "name", sizeof(char[strlen("name")]));
  char *description = request->body.search(&request->body, "description", sizeof(char[strlen("description")]));
  char *avatar = request->body.search(&request->body, "avatar", sizeof(char[strlen("avatar")]));

  if (code == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

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
  user_free(user);
  return format_200_with_content_type(response, "application/json");
}

/**
 * It gets the group members for a group
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A list of members in a group.
 */
char *get_group_members(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->query.search(&request->query, "code", sizeof(char[strlen("code")]));

  if (code == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (group->owner_id != user->id)
  {
    return format_403();
  }

  struct LinkedList *members = group->get_members(group);
  char *response = members->to_json(members, (char *(*)(void *))user->to_json);

  group_free(group);
  user_free(user);

  return format_200_with_content_type(response, "application/json");
}

/**
 * It deletes a group
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *delete_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->body.search(&request->body, "code", sizeof(char[strlen("code")]));

  if (code == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (group->owner_id != user->id)
  {
    return format_403();
  }

  if (group->remove(group))
  {
    return format_500();
  }

  group_free(group);
  user_free(user);

  return format_200();
}

/**
 * It takes a user's request to join a group, and if the user is authenticated, the group exists, and
 * the user isn't already a member, it adds the user to the group
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A string.
 */
char *join_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->body.search(&request->body, "code", sizeof(char[strlen("code")+1]));

  if (code == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (group->is_member(group, user))
  {
    return format_403();
  }

  if (group->add_member(group, user))
  {
    return format_500();
  }

  char *response = group->to_json(group);

  group_free(group);
  user_free(user);

  return format_200_with_content_type(response, "application/json");
}

/**
 * It removes a user from a group
 * 
 * @param server The server that is handling the request.
 * @param request The HTTPRequest struct that contains the request information.
 * 
 * @return A string.
 */
char *leave_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->body.search(&request->body, "code", sizeof(char[strlen("code")]));

  if (code == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (!group->is_member(group, user))
  {
    return format_403();
  }

  if (group->remove_member(group, user))
  {
    return format_500();
  }

  group_free(group);
  user_free(user);

  return format_200();
}

/**
 * It kicks a member from a group
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A string containing the response body.
 */
char *kick_member_group(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char *code = request->body.search(&request->body, "code", sizeof(char[strlen("code")]));
  char *member_id = request->body.search(&request->body, "member_id", sizeof(char[strlen("member_id")]));

  if (code == NULL || member_id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_code(code);

  if (group == NULL)
  {
    return format_404();
  }

  if (group->owner_id != user->id)
  {
    return format_403();
  }

  struct User *member = user_find_by_id(atoi(member_id));

  if (member == NULL)
  {
    return format_404();
  }

  if (!group->is_member(group, member))
  {
    return format_400();
  }

  if (group->remove_member(group, member))
  {
    return format_500();
  }

  group_free(group);
  user_free(user);
  user_free(member);

  return format_200();
}

char *get_my_groups(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  struct User *user = get_user_from_request(request, NULL);

  if (user == NULL)
  {
    return format_401();
  }

  struct LinkedList *groups = group_find_by_member(user->id);
  char *response = groups->to_json(groups, (char *(*)(void *))group_to_json);
  user_free(user);
  linked_list_destructor(groups, (void (*)(void *))group_free);
  return format_200_with_content_type(response, "application/json");
}