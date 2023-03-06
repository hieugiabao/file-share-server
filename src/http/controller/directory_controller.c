
#include "model/directory.h"
#include "http/controller/directory_controller.h"
#include "http/helper/helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *node_to_json(void *node);

/**
 * It creates a new directory
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A pointer to a string.
 */
char *make_directory(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *name = request->body.search(&request->body, "name", 5);
  char *group_id = request->body.search(&request->body, "group_id", 9);
  char *parent_id = request->body.search(&request->body, "parent_id", 10);

  if (name == NULL || group_id == NULL)
  {
    return format_422();
  }

  struct Group *group = group_find_by_id(atol(group_id));
  if (group == NULL)
  {
    return format_404();
  }
  
  long *parent_id_ptr = NULL;
  if (parent_id != NULL)
  {
    parent_id_ptr = malloc(sizeof(long));
    *parent_id_ptr = atol(parent_id);
    if (group->has_directory(group, *parent_id_ptr) != 1)
    {
      return format_403(); // The user is not allowed to create a directory in the parent directory
    }
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }
  if (group->is_member(group, user) != 1)
  {
    return format_403();
  }

  struct Directory *directory = directory_new(
      name,
      user->id,
      group->id,
      parent_id_ptr);
  free(parent_id_ptr);

  if (directory == NULL)
  {
    user_free(user);
    group_free(group);
    return format_500();
  }

  if (directory->save(directory) != 0)
  {
    directory_free(directory);
    user_free(user);
    group_free(group);
    return format_500();
  }

  char *json = directory->to_json(directory);
  directory_free(directory);
  user_free(user);
  group_free(group);

  return format_200_with_content_type(json, "application/json");
}

/**
 * It deletes a directory
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A pointer to a string.
 */
char *delete_directory(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *id = request->body.search(&request->body, "id", 3);
  if (id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct Directory *directory = directory_find_by_id(atol(id));
  if (directory == NULL)
  {
    return format_404();
  }

  if (directory->owner_id != user->id)
  {
    return format_403();
  }

  if (directory->remove(directory) != 0)
  {
    return format_500();
  }

  return format_200();
}

/**
 * It updates a directory
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A pointer to a string.
 */
char *update_directory(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *id = request->body.search(&request->body, "directory_id", 3);
  char *permission = request->body.search(&request->body, "permission", 11);

  if (id == NULL || permission == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct Directory *directory = directory_find_by_id(atol(id));
  if (directory == NULL)
  {
    return format_404();
  }

  if (directory->owner_id != user->id)
  {
    return format_403();
  }
  directory->permission = atol(permission);

  if (directory->update(directory) != 0)
  {
    directory_free(directory);
    user_free(user);
    return format_500();
  }

  char *json = directory->to_json(directory);
  directory_free(directory);
  user_free(user);

  return format_200_with_content_type(json, "application/json");
}

/**
 * It gets the children of a directory
 * 
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A pointer to a string.
 */
char *get_directory_children(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *id = request->body.search(&request->body, "directory_id", 13);
  if (id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct Directory *directory = directory_find_by_id(atol(id));
  if (directory == NULL)
  {
    return format_404();
  }

  if (directory->get_group(directory)->is_member(directory->get_group(directory), user) != 1)
  {
    return format_403();
  }

  struct LinkedList *children = directory->get_children(directory);
  if (children == NULL)
  {
    return format_500();
  }

  char *json = children->to_json(children, node_to_json);

  user_free(user);
  directory_free(directory);

  return format_200_with_content_type(json, "application/json");
}

/**
 * It takes a pointer to a node, and returns a pointer to a string that contains a JSON representation
 * of the node
 * 
 * @param node the node to be converted to json
 * 
 * @return A string representation of the node.
 */
char *node_to_json(void *node)
{
  struct FNode *n = (struct FNode *)node;
  char *fjson = NULL;
  switch (n->type)
  {
  case FDIRECTORY:
    char *dir_js = n->node.directory->to_json(n->node.directory);
    fjson = malloc(strlen(dir_js) + 35);
    sprintf(fjson, "{\"node\":%s, \"type\": \"directory\"}", dir_js);
    free(dir_js);
    break;
  case FFILE:
    char *file_js = n->node.file->to_json(n->node.file);
    fjson = malloc(strlen(file_js) + 30);
    sprintf(fjson, "{\"node\":%s, \"type\": \"file\"}", file_js);
    free(file_js);
    break;
  default:
    fjson = malloc(3);
    strcpy(fjson, "{}");
    break;
  }

  return fjson;
}

char *get_group_node_tree(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *id = request->body.search(&request->body, "group_id", 9);
  if (id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_id(atol(id));
  if (group == NULL)
  {
    return format_404();
  }

  if (group->is_member(group, user) != 1)
  {
    return format_403();
  }

  struct LinkedList *children = get_root_node_by_group(group->id);
  if (children == NULL)
  {
    return format_500();
  }

  char *json = children->to_json(children, node_to_json);

  user_free(user);
  group_free(group);
  linked_list_destructor(children, NULL);

  return format_200_with_content_type(json, "application/json");
}