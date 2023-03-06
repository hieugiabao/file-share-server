#include "model/file.h"
#include "database/db.h"
#include "model/directory.h"
#include "utils/helper.h"
#include "setting.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Private methods prototype */

int file_save(struct File *file);
int file_remove(struct File *file);
int file_update(struct File *file);
struct User *file_get_owner(struct File *file);
struct User *file_get_modified_user(struct File *file);
struct Group *file_get_group(struct File *file);
struct Directory *file_get_directory(struct File *file);

void _get_file_callback(sqlite3_stmt *res, void *arg);

/* Public methods implementation */

/**
 * It creates a new file object and returns it
 *
 * @param fullname The full name of the file, including the path.
 * @param size The size of the file in bytes
 * @param user_id The user id of the user who created the file
 * @param group_id The group id of the file
 * @param directory_id The id of the directory that the file is in.
 *
 * @return A pointer to a struct File
 */
struct File *
file_new(char *fullname, long size, long user_id, long group_id, long *directory_id)
{
  struct File *file = malloc(sizeof(struct File));
  file->name = strdup(fullname);
  file->size = size;
  file->permission = READ;
  file->path = NULL;
  file->directory_id = directory_id == NULL ? 0 : *directory_id;
  file->group_id = group_id;
  file->owner_id = user_id;
  file->modified_by = user_id;
  file->created_at = NULL;
  file->updated_at = NULL;

  file->save = file_save;
  file->remove = file_remove;
  file->update = file_update;
  file->get_owner = file_get_owner;
  file->get_modified_user = file_get_modified_user;
  file->get_group = file_get_group;
  file->get_directory = file_get_directory;
  file->to_json = file_to_json;

  return file;
}

/**
 * It frees the memory allocated for the file struct
 *
 * @param file The file object to free.
 */
void file_free(struct File *file)
{
  if (file->_directory != NULL)
    directory_free(file->_directory);
  if (file->_group != NULL)
    group_free(file->_group);
  if (file->_owner != NULL)
    user_free(file->_owner);
  if (file->_modified_by != NULL)
    user_free(file->_modified_by);
  free(file->name);
  free(file->path);
  free(file->created_at);
  free(file->updated_at);
  free(file);
}

/**
 * It finds a file by its id and returns it.
 *
 * @param id The id of the file to find.
 *
 * @return A pointer to a struct File
 */
struct File *file_find_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char query[] = "SELECT * FROM files WHERE id = ?";

  struct File *file = NULL;
  int res = pool->exec(pool, _get_file_callback, &file, query, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
  {
    return NULL;
  }

  return file;
}

/**
 * It takes a pointer to a File struct and returns a JSON string representation of the struct
 *
 * @param file The file object to be converted to JSON
 *
 * @return A JSON string.
 */
char *file_to_json(struct File *file)
{
  char *json = malloc(4096);
  sprintf(json, "{\"id\":%ld,\"name\":\"%s\",\"size\":%ld,\"permission\":%d,\"path\":\"%s\",\"directory_id\":%ld,\"group_id\":%ld,\"owner_id\":%ld,\"modified_by\":%ld,\"created_at\":\"%s\",\"updated_at\":\"%s\"}",
          file->id, file->name, file->size, file->permission, file->path, file->directory_id, file->group_id, file->owner_id, file->modified_by, file->created_at, file->updated_at);
  return json;
}

/**
 * It saves a file to the database
 *
 * @param file The file object to save
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most
 * recent SQL statement.
 */
int file_save(struct File *file)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  struct Directory *directory = file->get_directory(file);
  if (directory != NULL)
  {
    file->permission = directory->permission;
    file->path = malloc(strlen(directory->path) + strlen(file->name) + 2);
    sprintf(file->path, "%s/%s", directory->path, file->name);
  }
  else
  {
    struct Group *group = file->get_group(file);
    file->path = malloc(strlen(file->name) + strlen(UPLOAD_DIR) + strlen(group->code) + 3);
    sprintf(file->path, "%s/%s/%s", UPLOAD_DIR, group->code, file->name);
  }

  char query[] = "INSERT INTO files (name, size, permission, path, directory_id, group_id, owner_id, modified_by) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

  int res = pool->exec(pool, NULL, NULL, query, 8, file->name,
                       convert_long_to_string(file->size), convert_long_to_string(file->permission),
                       file->path, file->directory_id != 0 ? convert_long_to_string(file->directory_id) : NULL,
                       convert_long_to_string(file->group_id), convert_long_to_string(file->owner_id),
                       convert_long_to_string(file->modified_by));

  if (res != SQLITE_OK)
  {
    return -1;
  }
  file->id = sqlite3_last_insert_rowid(pool->db);

  return 0;
}

/**
 * It removes a file from the database
 *
 * @param file The file object to remove
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most
 * recent SQL statement.
 */
int file_remove(struct File *file)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char query[] = "DELETE FROM files WHERE id = ?";

  int res = pool->exec(pool, NULL, NULL, query, 1, convert_long_to_string(file->id));

  if (res != SQLITE_OK)
  {
    return -1;
  }
  // delete file from disk
  remove(file->path);

  return 0;
}

/**
 * It updates a file in the database
 *
 * @param file The file object to update
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most
 * recent SQL statement.
 */
int file_update(struct File *file)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *updated_at = get_current_time();

  char query[] = "UPDATE files SET size = ?, permission = ?, modified_by = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";

  int res = pool->exec(pool, NULL, NULL, query, 4,
                       convert_long_to_string(file->size),
                       convert_long_to_string(file->permission),
                       convert_long_to_string(file->modified_by),
                       convert_long_to_string(file->id));

  if (res != SQLITE_OK)
  {
    return -1;
  }
  file->updated_at = strdup(updated_at);

  return 0;
}

/**
 * It returns the directory of the file
 *
 * @param file The file object
 *
 * @return A pointer to a struct Directory
 */
struct Directory *file_get_directory(struct File *file)
{
  if (file->directory_id == 0)
    return NULL;
  if (file->_directory == NULL)
  {
    file->_directory = directory_find_by_id(file->directory_id);
  }

  return file->_directory;
}

/**
 * It returns the group of the file
 *
 * @param file The file object
 *
 * @return A pointer to a struct Group
 */
struct Group *file_get_group(struct File *file)
{
  if (file->_group == NULL)
  {
    file->_group = group_find_by_id(file->group_id);
  }

  return file->_group;
}

/**
 * It returns the owner of the file
 *
 * @param file The file object
 *
 * @return A pointer to a struct User
 */
struct User *file_get_owner(struct File *file)
{
  if (file->_owner == NULL)
  {
    file->_owner = user_find_by_id(file->owner_id);
  }

  return file->_owner;
}

/**
 * It returns the user who modified the file
 *
 * @param file The file object
 *
 * @return A pointer to a struct User
 */
struct User *file_get_modified_user(struct File *file)
{
  if (file->_modified_by == NULL)
  {
    file->_modified_by = user_find_by_id(file->modified_by);
  }

  return file->_modified_by;
}

/**
 * It takes the result of a query and turns it into a `File` object
 *
 * @param res The result of the query.
 * @param arg A pointer to the variable that will hold the result.
 */
void _get_file_callback(sqlite3_stmt *res, void *arg)
{
  long *directory_id = NULL;
  if (sqlite3_column_type(res, 5) != SQLITE_NULL)
  {
    directory_id = malloc(sizeof(long));
    *directory_id = sqlite3_column_int64(res, 3);
  }
  struct File *file = file_new(
      (char *)sqlite3_column_text(res, 1), // name
      sqlite3_column_int64(res, 2),        // size
      sqlite3_column_int64(res, 7),        // user_id
      sqlite3_column_int64(res, 6),        // group_id
      directory_id                         // directory_id
  );
  file->id = sqlite3_column_int64(res, 0);
  file->permission = sqlite3_column_int64(res, 3);
  file->path = strdup((char *)sqlite3_column_text(res, 4));
  file->modified_by = sqlite3_column_int64(res, 8);
  file->created_at = strdup((char *)sqlite3_column_text(res, 9));
  file->updated_at = strdup((char *)sqlite3_column_text(res, 10));

  *(struct File **)arg = file;
}