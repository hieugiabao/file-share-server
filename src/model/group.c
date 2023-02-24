#include "model/group.h"
#include "database/db.h"
#include "utils/helper.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Public member function prototypes */

int group_save(struct Group *group);
int group_update(struct Group *group);
int group_remove(struct Group *group);
int group_add_member(struct Group *group, struct User *user);
int group_leave(struct Group *group, struct User *user);
struct User *group_get_owner(struct Group *group);
struct LinkedList *group_get_members(struct Group *group);

/* Private helper function prototypes */
void _retreive_code_callback(sqlite3_stmt *res, void *arg);
void _get_group_members_callback(sqlite3_stmt *res, void *arg);
void _get_group_callback(sqlite3_stmt *res, void *arg);
void _get_groups_callback(sqlite3_stmt *res, void *arg);

/* Public member functions */

/**
 * It creates a new group and returns a pointer to it
 *
 * @param name The name of the group
 * @param description A short description of the group
 * @param avatar The avatar of the group.
 * @param owner The user who created the group.
 *
 * @return A pointer to a struct Group
 */
struct Group *
group_new(char *name, char *description, char *avatar, long owner_id)
{
  struct Group *group = malloc(sizeof(struct Group));
  group->name = strdup(name);
  if (description == NULL)
    group->description = NULL;
  else
    group->description = strdup(description);
  if (avatar == NULL)
    group->avatar = NULL;
  else
    group->avatar = strdup(avatar);

  group->owner_id = owner_id;
  group->status = 1;

  group->save = group_save;
  group->update = group_update;
  group->remove = group_remove;
  group->add_member = group_add_member;
  group->remove_member = group_leave;
  group->get_owner = group_get_owner;
  group->get_members = group_get_members;
  group->to_json = group_to_json;

  group->_owner = NULL;
  group->_members = NULL;

  return group;
}

/**
 * It frees all the memory allocated to a group
 *
 * @param group The group to free.
 */
void group_free(struct Group *group)
{
  free(group->name);
  free(group->description);
  free(group->avatar);
  free(group->code);

  if (group->_owner != NULL)
  {
    user_free(group->_owner);
  }

  if (group->_members != NULL)
  {
    linked_list_destructor(group->_members, (void (*)(void *))user_free);
  }
  free(group);
}

/**
 * It saves a group to the database
 *
 * @param group The group to save.
 *
 * @return 0 if the group was saved successfully, -1 otherwise.
 */
int group_save(struct Group *group)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *query = "INSERT INTO groups (name, description, avatar, owner_id, status) VALUES (?, ?, ?, ?, ?)";
  int res = pool->exec(pool, NULL, NULL, query, 5, group->name, group->description, group->avatar, convert_long_to_string(group->owner_id), convert_int_to_string(group->status));

  if (res != SQLITE_OK)
    return -1;

  int last_id = sqlite3_last_insert_rowid(pool->db);
  group->id = last_id;

  // retrive the group code
  query = "SELECT code FROM groups WHERE id = ?";
  char *code = NULL;
  res = pool->exec(pool, _retreive_code_callback, &code, query, 1, convert_long_to_string(group->id));

  if (res != SQLITE_OK)
    return -1;
  group->code = strdup(code);
  free(code);

  query = "INSERT INTO group_members (group_id, user_id) VALUES (?, ?)";
  res = pool->exec(pool, NULL, NULL, query, 2, convert_long_to_string(group->id), convert_long_to_string(group->owner_id));
  if (res != SQLITE_OK)
  {
    // rollback
    query = "DELETE FROM groups WHERE id = ?";
    pool->exec(pool, NULL, NULL, query, 1, convert_long_to_string(group->id));
    return -1;
  }
  return 0;
}

/**
 * It updates a group in the database
 *
 * @param group The group to update.
 *
 * @return The return value is the number of rows that were changed or inserted or deleted by the most recently completed
 * SQL statement on the database connection specified by the first parameter.
 */
int group_update(struct Group *group)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *query = "UPDATE groups SET name = ?, description = ?, avatar = ?, status = ? WHERE id = ?";
  int res = pool->exec(pool, NULL, NULL, query, 5, group->name, group->description, group->avatar, convert_int_to_string(group->status), convert_long_to_string(group->id));

  if (res != SQLITE_OK)
    return -1;

  return 0;
}

/**
 * It deletes a group from the database
 *
 * @param group The group to remove.
 *
 * @return The return value is the number of rows that were changed.
 */
int group_remove(struct Group *group)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *query = "DELETE FROM groups WHERE id = ?";

  int res = pool->exec(pool, NULL, NULL, query, 1, convert_long_to_string(group->id));

  if (res != SQLITE_OK)
    return -1;

  return 0;
}

/**
 * It adds a user to a group
 *
 * @param group The group to add the user to.
 * @param user The user to add to the group
 *
 * @return The return value is the number of rows that were changed by the SQL statement.
 */
int group_add_member(struct Group *group, struct User *user)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *query = "INSERT INTO group_members (group_id, user_id) VALUES (?, ?)";

  int res = pool->exec(pool, NULL, NULL, query, 2, convert_long_to_string(group->id), convert_long_to_string(user->id));

  if (res != SQLITE_OK)
    return -1;

  return 0;
}

/**
 * It removes a user from a group
 *
 * @param group The group to remove the user from.
 * @param user The user to remove from the group.
 *
 * @return The return value is the number of rows that were changed by the SQL statement.
 */
int group_leave(struct Group *group, struct User *user)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *query = "DELETE FROM group_members WHERE group_id = ? AND user_id = ?";

  int res = pool->exec(pool, NULL, NULL, query, 2, convert_long_to_string(group->id), convert_long_to_string(user->id));

  if (res != SQLITE_OK)
    return -1;

  return 0;
}

/**
 * "Get the owner of a group."
 *
 * The first thing we do is check if the owner has already been loaded. If it has, we return it
 *
 * @param group The group to get the owner of.
 *
 * @return A pointer to a User struct.
 */
struct User *group_get_owner(struct Group *group)
{
  if (group->_owner != NULL)
    return group->_owner;
  struct User *user = user_find_by_id(group->owner_id);

  group->_owner = user;
  return user;
}

/**
 * It gets the members of a group
 *
 * @param group The group to get the members of.
 *
 * @return A linked list of user ids.
 */
struct LinkedList *group_get_members(struct Group *group)
{
  if (group->_members != NULL)
    return group->_members;

  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT user_id FROM group_members WHERE group_id = ?";

  struct LinkedList members = linked_list_constructor();
  struct LinkedList *members_ptr = malloc(sizeof(struct LinkedList));
  *members_ptr = members;
  int res = pool->exec(pool, _get_group_members_callback, members_ptr, query, 1, convert_long_to_string(group->id));

  if (res != SQLITE_OK)
    return NULL;

  group->_members = members_ptr;
  return group->_members;
}

/**
 * It takes a group and returns a JSON string representing that group
 *
 * @param group The group to convert to JSON.
 *
 * @return A JSON string representing the group.
 */
char *group_to_json(struct Group *group)
{
  struct User *owner = group_get_owner(group);
  char *owner_json = owner->to_json(owner);
  char *json = malloc(sizeof(char) * 1024);
  sprintf(json, "{\"id\":%ld,\"name\":\"%s\",\"description\":\"%s\",\"avatar\":\"%s\",\"status\":%d,\"code\": \"%s\",\"owner\":%s}",
          group->id, group->name,
          group->description ? group->description : "",
          group->avatar ? group->avatar : "",
          group->status,
          group->code,
          owner_json);
  return json;
}

/* Private helper function implements */

/**
 * It retrieves the code from the database and stores it in the variable pointed to by the argument
 *
 * @param res The result of the query.
 * @param arg The argument passed to the callback function.
 */
void _retreive_code_callback(sqlite3_stmt *res, void *arg)
{
  char *code = (char *)sqlite3_column_text(res, 0);
  *(char **)arg = strdup(code);
}

/**
 * It takes a group id, and returns a linked list of all the members of that group
 *
 * @param res The result of the query.
 * @param arg The argument passed to the callback function.
 */
void _get_group_members_callback(sqlite3_stmt *res, void *arg)
{
  struct LinkedList *members = (struct LinkedList *)arg;

  long user_id = sqlite3_column_int64(res, 0);

  struct User *user = user_find_by_id(user_id);
  members->insert(members, 0, user, sizeof(struct User));
}

void _get_group_callback(sqlite3_stmt *res, void *arg)
{
  struct Group *group = group_new(
      (char *)sqlite3_column_text(res, 1),
      (char *)sqlite3_column_text(res, 2),
      (char *)sqlite3_column_text(res, 3),
      sqlite3_column_int64(res, 5));
  group->id = sqlite3_column_int64(res, 0);

  group->status = sqlite3_column_int(res, 4);
  group->code = strdup((char *)sqlite3_column_text(res, 6));

  *(struct Group **)arg = group;
}

void _get_groups_callback(sqlite3_stmt *res, void *arg)
{
  struct LinkedList *groups = (struct LinkedList *)arg;
  struct Group *group = NULL;
  _get_group_callback(res, &group);
  groups->insert(groups, 0, group, sizeof(struct Group));
}

/* Public function implements */

/**
 * It gets a group by its id
 *
 * @param id The id of the group to get.
 *
 * @return A group struct
 */
struct Group *group_find_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT id, name, description, avatar, status, owner_id. code FROM groups WHERE id = ?";

  struct Group *group = NULL;

  int res = pool->exec(pool, _get_group_callback, &group, query, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
    return NULL;

  return group;
}

/**
 * It gets a group by name
 *
 * @param name The name of the group to get.
 *
 * @return A group struct
 */
struct Group *group_find_by_name(char *name)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT id, name, description, avatar, status, owner_id, code FROM groups WHERE name = ?";

  struct Group *group = NULL;

  int res = pool->exec(pool, _get_group_callback, &group, query, 1, name);

  if (res != SQLITE_OK)
    return NULL;

  return group;
}

/**
 * It gets a group by its code
 *
 * @param code The group code
 *
 * @return A group struct
 */
struct Group *group_find_by_code(char *code)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT id, name, description, avatar, status, owner_id, code FROM groups WHERE code = ?";

  struct Group *group = NULL;

  int res = pool->exec(pool, _get_group_callback, &group, query, 1, code);

  if (res != SQLITE_OK)
    return NULL;

  return group;
}

struct LinkedList *group_find_by_member(long member_id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *query = "SELECT groups.id, name, description, avatar, status, owner_id, code FROM groups INNER JOIN group_members ON groups.id = group_members.group_id WHERE user_id = ?";

  struct LinkedList groups = linked_list_constructor();
  struct LinkedList *groups_ptr = malloc(sizeof(struct LinkedList));
  *groups_ptr = groups;
  int res = pool->exec(pool, _get_groups_callback, groups_ptr, query, 1, convert_long_to_string(member_id));

  if (res != SQLITE_OK)
    return NULL;

  return groups_ptr;
}