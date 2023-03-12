
#include "http/controller/file_controller.h"
#include "model/file.h"
#include "http/helper/helper.h"
#include "setting.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * It creates a file
 *
 * @param server The server object
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *create_file(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *name = request->body.search(&request->body, "name", 5);
  // char *size = request->body.search(&request->body, "size", 5);
  char *group_id = request->body.search(&request->body, "group_id", 9);
  char *directory_id = request->body.search(&request->body, "directory_id", 13);
  char path[1024];

  if (name == NULL || group_id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct Group *group = group_find_by_id(atol(group_id));
  if (group == NULL)
  {
    return format_404();
  }

  if (group->is_member(group, user) != 1)
  {
    group_free(group);
    user_free(user);
    return format_403();
  }

  long *directory_id_ptr = NULL;
  if (directory_id != NULL)
  {
    directory_id_ptr = malloc(sizeof(long));
    *directory_id_ptr = atol(directory_id);
    struct Directory *directory = directory_find_by_id(*directory_id_ptr);
    if (directory == NULL || group->has_directory(group, *directory_id_ptr) != 1 || (directory->permission == READ && (user->id != directory->owner_id || user->id != group->owner_id)))
    {
      group_free(group);
      user_free(user);
      directory_free(directory);
      return format_403(); // The user is not allowed to create a file in the directory
    }
    sprintf(path, "%s/%s", directory->path, name);
    directory_free(directory);
  }
  else
  {
    sprintf(path, "%s/%s", group->code, name);
  }

  struct File *file = file_find_by_name(name, group->id, directory_id_ptr);
  if (file != NULL)
  {
    group_free(group);
    user_free(user);
    return format_409();
  }

  char json[4096];
  sprintf(json, "{\"path\": \"%s\"}", path);

  user_free(user);
  group_free(group);
  free(directory_id_ptr);

  return format_200_with_content_type(json, "application/json");
}

/**
 * It deletes a file
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *delete_file(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *file_id = request->body.search(&request->body, "file_id", 8);
  if (file_id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct File *file = file_find_by_id(atol(file_id));
  if (file == NULL)
  {
    user_free(user);
    return format_400();
  }

  struct Group *group = group_find_by_id(file->group_id);
  if (group == NULL)
  {
    user_free(user);
    file_free(file);
    return format_400();
  }

  if (file->owner_id != user->id && group->owner_id != user->id) //
  {
    user_free(user);
    group_free(group);
    file_free(file);
    return format_403();
  }

  if (file->remove(file) != 0)
  {
    user_free(user);
    group_free(group);
    file_free(file);
    return format_500();
  }

  user_free(user);
  group_free(group);
  file_free(file);

  return format_200();
}

/**
 * It updates a file
 * 
 * @param server the server object
 * @param request The HTTPRequest object that contains the request information.
 * 
 * @return A file object in JSON format.
 */
char *update_file(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *file_id = request->body.search(&request->body, "file_id", 8);
  char *size = request->body.search(&request->body, "size", 5);
  char *permission = request->body.search(&request->body, "permission", 11);

  if (file_id == NULL || (size == NULL && permission == NULL))
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct File *file = file_find_by_id(atol(file_id));
  if (file == NULL)
  {
    user_free(user);
    return format_400();
  }

  struct Group *group = group_find_by_id(file->group_id);
  if (group == NULL)
  {
    user_free(user);
    file_free(file);
    return format_400();
  }

  if (group->is_member(group, user) != 1 && (file->permission != WRITE || (file->owner_id != user->id && group->owner_id != user->id))) //
  {
    user_free(user);
    group_free(group);
    file_free(file);
    return format_403();
  }

  if (size != NULL)
    file->size = atol(size);
  if (permission != NULL)
    file->permission = atoi(permission);
  file->modified_by = user->id;

  if (file->update(file) != 0)
  {
    return format_500();
  }

  char *json = file->to_json(file);

  file_free(file);
  user_free(user);
  group_free(group);

  return format_200_with_content_type(json, "application/json");
}

char *get_file(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *file_id = request->body.search(&request->body, "file_id", 8);
  if (file_id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  struct File *file = file_find_by_id(atol(file_id));
  if (file == NULL)
  {
    user_free(user);
    return format_400();
  }

  struct Group *group = group_find_by_id(file->group_id);
  if (group == NULL)
  {
    user_free(user);
    file_free(file);
    return format_400();
  }

  if (group->is_member(group, user) != 1) //
  {
    user_free(user);
    group_free(group);
    file_free(file);
    return format_403();
  }

  char *json = file->to_json(file);

  file_free(file);
  user_free(user);
  group_free(group);

  return format_200_with_content_type(json, "application/json");
}

char *save_file(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *name = request->body.search(&request->body, "name", 5);
  char *size = request->body.search(&request->body, "size", 5);
  char *path = request->body.search(&request->body, "path", 5);
  char *group_id = request->body.search(&request->body, "group_id", 9);
  char *directory_id = request->body.search(&request->body, "directory_id", 13);

  if (name == NULL || size == NULL || path == NULL || group_id == NULL)
  {
    return format_422();
  }

  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }
  char fullpath[1024];
  sprintf(fullpath, "%s/%s", UPLOAD_DIR, path);

  // check file path exists
  if (access(fullpath, F_OK) != 0)
  {
    printf("Not oke: %s\n", fullpath);
    return format_400();
  }

  struct File *file = file_new(
      name,
      atol(size),
      user->id,
      atol(group_id),
      NULL);
  file->path = strdup(path);
  file->directory_id = directory_id == NULL ? 0 : atol(directory_id);
  if (file->save(file) != 0)
  {
    // remvoe file
    remove(fullpath);

    user_free(user);
    file_free(file);
    return format_500();
  }

  char *json = file->to_json(file);

  user_free(user);
  file_free(file);

  return format_200_with_content_type(json, "application/json");
}