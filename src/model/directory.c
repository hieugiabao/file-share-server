#include "model/directory.h"
#include "database/db.h"
#include "model/file.h"
#include "setting.h"
#include "logger/logger.h"
#include "utils/helper.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private method prototypes */

int directory_save(struct Directory *directory);
int directory_remove(struct Directory *directory);
int directory_update(struct Directory *directory);
struct User *directory_get_owner(struct Directory *directory);
struct Group *directory_get_group(struct Directory *directory);
struct Directory *directory_get_parent(struct Directory *directory);
struct LinkedList *directory_get_children(struct Directory *directory);
char *directory_json(struct Directory *directory);

void _get_directory_callback(sqlite3_stmt *res, void *arg);
void _get_directories_callback(sqlite3_stmt *res, void *arg);
void _get_files_callback(sqlite3_stmt *res, void *arg);
struct LinkedList *_directory_get_children_by_id(long id);
void _file_node_free(void *node);

/* Public methods implements */

/**
 * It creates a new directory object and returns it
 *
 * @param name The name of the directory
 * @param user_id The user id of the owner of the directory
 * @param group_id The group id of the directory
 * @param parent_id The id of the parent directory.
 *
 * @return A pointer to a struct Directory
 */
struct Directory *
directory_new(const char *name, long user_id, long group_id, long *parent_id)
{
  struct Directory *directory = (struct Directory *)malloc(sizeof(struct Directory));
  directory->name = strdup(name);
  directory->owner_id = user_id;
  directory->group_id = group_id;
  directory->parent_id = parent_id != NULL ? *parent_id : 0;
  directory->permission = READ;

  directory->save = directory_save;
  directory->remove = directory_remove;
  directory->update = directory_update;
  directory->get_owner = directory_get_owner;
  directory->get_group = directory_get_group;
  directory->get_parent = directory_get_parent;
  directory->get_children = directory_get_children;
  directory->to_json = directory_json;

  directory->path = NULL;
  directory->_group = NULL;
  directory->_children = NULL;
  directory->_parent = NULL;
  directory->_owner = NULL;

  return directory;
}

/**
 * It frees the memory allocated for a directory
 *
 * @param directory The directory to free.
 */
void directory_free(struct Directory *directory)
{
  if (directory->_owner != NULL)
    user_free(directory->_owner);
  if (directory->_group != NULL)
    group_free(directory->_group);
  if (directory->_parent != NULL)
    directory_free(directory->_parent);
  if (directory->_children != NULL)
    linked_list_destructor(directory->_children, _file_node_free);
  free(directory->name);
  free(directory->path);
  free(directory->created_at);
  free(directory->updated_at);
  free(directory);
}

/**
 * It gets a directory from the database by its id
 *
 * @param id The id of the directory to get
 *
 * @return A pointer to a struct Directory
 */
struct Directory *directory_find_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char query[] = "SELECT * FROM directories WHERE id = ?";

  struct Directory *directory = NULL;
  int res = pool->exec(pool, _get_directory_callback, &directory, query, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
  {
    return NULL;
  }

  return directory;
}

/* Private methods implements */

/**
 * It saves a directory to the database
 *
 * @param directory The directory to save
 *
 * @return The id of the directory
 */
int directory_save(struct Directory *directory)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;
  
  // find path
  if (directory->parent_id != 0)
  {
    struct Directory *parent = directory_find_by_id(directory->parent_id);
    if (parent == NULL)
    {
      return -1;
    }

    directory->permission = parent->permission;
    directory->path = malloc(strlen(parent->path) + strlen(directory->name) + 2);
    sprintf(directory->path, "%s/%s", parent->path, directory->name);
    directory_free(parent);
  }
  else
  {
    struct Group *group = directory->get_group(directory);
    if (group == NULL)
    {
      return -1;
    }
    directory->path = malloc(strlen(directory->name) + strlen(group->code) + 2);
    sprintf(directory->path, "%s/%s", group->code, directory->name);
  }
  char fullpath[1024];
  sprintf(fullpath, "%s/%s", UPLOAD_DIR, directory->path);
  if (create_directory(fullpath))
  {
    return -1;
  }

  char query[] = "INSERT INTO directories (name, owner_id, group_id, parent_id, path, permission) VALUES (?, ?, ?, ?, ?, ?)";

  int res = pool->exec(pool, NULL, NULL, query, 6, directory->name,
                       convert_long_to_string(directory->owner_id), convert_long_to_string(directory->group_id),
                       directory->parent_id != 0 ? convert_long_to_string(directory->parent_id) : NULL,
                       directory->path,
                       convert_int_to_string(directory->permission));

  if (res != SQLITE_OK)
  {
    // remove dir
    remove_directory(directory->path);
    return -1;
  }
  directory->id = sqlite3_last_insert_rowid(pool->db);

  return 0;
}

/**
 * It removes a directory from the database
 *
 * @param directory The directory to remove
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most
 * recent SQL statement.
 */
int directory_remove(struct Directory *directory)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char query[] = "DELETE FROM directories WHERE id = ?";

  int res = pool->exec(pool, NULL, NULL, query, 1, convert_long_to_string(directory->id));

  if (res != SQLITE_OK)
  {
    return -1;
  }

  char fullpath[1024];
  sprintf(fullpath, "%s/%s", UPLOAD_DIR, directory->path);
  remove_directory(fullpath);

  return 0;
}

/**
 * It updates a directory in the database
 *
 * @param directory The directory to update
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most
 * recently completed SQL statement on the database connection specified by the first parameter.
 */
int directory_update(struct Directory *directory)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char query[] = "UPDATE directories SET permission = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
  // find updated_at

  int res =
      pool->exec(pool, NULL, NULL, query, 2,
                 convert_int_to_string(directory->permission),
                 convert_long_to_string(directory->id));

  if (res != SQLITE_OK)
  {
    return -1;
  }
  directory->updated_at = get_current_time();

  return 0;
}

/**
 * If the directory's owner is not set, then find the owner by id and set it.
 * The function is a little more complicated than that, but that's the gist of it
 *
 * @param directory The directory to get the owner of.
 *
 * @return The owner of the directory.
 */
struct User *directory_get_owner(struct Directory *directory)
{
  if (directory->_owner == NULL)
  {
    directory->_owner = user_find_by_id(directory->owner_id);
  }
  return directory->_owner;
}

/**
 * If the directory's group is not set, then find the group by id and set it.
 * The function is a little more complicated than that, but that's the gist of it
 *
 * @param directory The directory to get the group of.
 *
 * @return The group of the directory.
 */
struct Group *directory_get_group(struct Directory *directory)
{
  printf("Into get group[directory]\n");
  if (directory->_group == NULL)
  {
    directory->_group = group_find_by_id(directory->group_id);
  }
  return directory->_group;
}

/**
 * If the directory's parent is not set, then find the parent by id and set it.
 * The function is a little more complicated than that, but that's the gist of it
 *
 * @param directory The directory to get the parent of.
 *
 * @return The parent of the directory.
 */
struct Directory *directory_get_parent(struct Directory *directory)
{
  if (directory->parent_id == 0)
    return NULL;
  if (directory->_parent == NULL)
  {
    directory->_parent = directory_find_by_id(directory->parent_id);
  }
  return directory->_parent;
}

/**
 * It returns the children of a directory
 *
 * @param directory The directory to get the children of.
 *
 * @return A pointer to a linked list of directories.
 */
struct LinkedList *directory_get_children(struct Directory *directory)
{
  if (directory->_children == NULL)
  {
    directory->_children = _directory_get_children_by_id(directory->id);
  }
  return directory->_children;
}

/**
 * It takes a `struct Directory` and returns a JSON string representation of it
 *
 * @param directory The directory object
 */
char *directory_json(struct Directory *directory)
{
  char *json = malloc(4096);
  sprintf(json, "{\"id\":%ld,\"name\":\"%s\",\"path\":\"%s\",\"owner_id\":%ld,\"parent_id\":%ld,\"permission\":%d,\"group_id\":%ld,\"created_at\":\"%s\",\"updated_at\":\"%s\"}",
          directory->id, directory->name, directory->path, directory->owner_id,
          directory->parent_id != 0 ? directory->parent_id : 0, directory->permission,
          directory->group_id, directory->created_at, directory->updated_at);

  return json;
}

struct LinkedList *get_root_node_by_group(long group_id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT * FROM directories WHERE parent_id IS NULL AND group_id = ?";

  struct LinkedList result = linked_list_constructor();
  struct LinkedList *res_ptr = malloc(sizeof(struct LinkedList));
  *res_ptr = result;
  int res = pool->exec(pool, _get_directories_callback, res_ptr, query, 1, convert_long_to_string(group_id));
  if (res != SQLITE_OK)
  {
    return NULL;
  }

  query = "SELECT * FROM files WHERE directory_id IS NULL AND group_id = ?";
  res = pool->exec(pool, _get_files_callback, res_ptr, query, 1, convert_long_to_string(group_id));
  if (res != SQLITE_OK)
  {
    return NULL;
  }

  return res_ptr;
}

/**
 * It frees the memory allocated to a file node
 * 
 * @param node The node to free.
 */
void _file_node_free(void *node)
{
  struct FNode *fnode = (struct FNode *)node;
  switch (fnode->type)
  {
  case FFILE:
    file_free(fnode->node.file);
    break;
  case FDIRECTORY:
    directory_free(fnode->node.directory);
    break;
  default:
    break;
  }
}

/**
 * It takes a row from the database and turns it into a `Directory` struct
 *
 * @param res The result of the query.
 * @param arg The argument passed to the callback function.
 */
void _get_directory_callback(sqlite3_stmt *res, void *arg)
{
  long *parent_id = NULL;
  if (sqlite3_column_type(res, 4) != SQLITE_NULL)
  {
    parent_id = malloc(sizeof(long));
    *parent_id = sqlite3_column_int64(res, 4);
  }
  struct Directory *directory = directory_new(
      (char *)sqlite3_column_text(res, 1), // name
      sqlite3_column_int64(res, 6),        // owner_id
      sqlite3_column_int64(res, 5),        // group_id
      parent_id);
  directory->id = sqlite3_column_int64(res, 0);
  directory->permission = sqlite3_column_int(res, 2);
  directory->path = strdup((char *)sqlite3_column_text(res, 3));
  directory->created_at = strdup((char *)sqlite3_column_text(res, 7));
  directory->updated_at = strdup((char *)sqlite3_column_text(res, 8));

  *((struct Directory **)arg) = directory;
}

/**
 * It takes a sqlite3_stmt, which is a result of a query, and a void pointer, which is a pointer to a
 * linked list, and it inserts a new node into the linked list, which is a directory
 *
 * @param res the result of the query
 * @param arg the argument passed to the callback function
 */
void _get_directories_callback(sqlite3_stmt *res, void *arg)
{
  struct LinkedList *list = (struct LinkedList *)arg;
  struct Directory *directory = NULL;
  _get_directory_callback(res, &directory);
  struct FNode *fnode = malloc(sizeof(struct FNode));
  fnode->node.directory = directory;
  fnode->type = FDIRECTORY;
  list->insert(list, list->length, fnode, sizeof(struct FNode)); // TODO: check if this works
}

/**
 * It takes a SQLite result set and a linked list, and it creates a new file object from the result set
 * and inserts it into the linked list
 *
 * @param res the result of the query
 * @param arg the list to insert the file into
 */
void _get_files_callback(sqlite3_stmt *res, void *arg)
{
  struct LinkedList *list = (struct LinkedList *)arg;
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

  struct FNode *fnode = malloc(sizeof(struct FNode));
  fnode->node.file = file;
  fnode->type = FFILE;

  list->insert(list, list->length, fnode, sizeof(struct FNode)); // TODO: check if this works
}

/**
 * It gets all the children of a directory by its id
 *
 * @param id The id of the directory to get the children of.
 *
 * @return A list of directories and files that are children of the directory with the given id.
 */
struct LinkedList *_directory_get_children_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  struct LinkedList list = linked_list_constructor();
  struct LinkedList *list_ptr = malloc(sizeof(struct LinkedList));
  *list_ptr = list;
  char *query = "SELECT * FROM directories WHERE parent_id = ? ORDER BY updated_at DESC";

  int res = pool->exec(pool, _get_directories_callback, list_ptr, query, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
  {
    return NULL;
  }

  query = "SELECT * FROM files WHERE directory_id = ? ORDER BY updated_at DESC";
  res = pool->exec(pool, _get_files_callback, list_ptr, query, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
  {
    return NULL;
  }
  return list_ptr;
}