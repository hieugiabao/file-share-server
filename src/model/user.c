
#include "model/user.h"
#include "database/db.h"
#include "logger/logger.h"
#include "utils/helper.h"

#include <string.h>

/* Private methods prototype */

int save_user(struct User *user);
int update_user(struct User *user);
int delete_user(struct User *user);
char *json_user(struct User *user);

void _find_user_callback(sqlite3_stmt *res, void *arg);

/* Public methods implements */

/**
 * It creates a new user object and returns a pointer to it
 *
 * @param display_name The user's display name.
 * @param username The username of the user.
 * @param password The password for the user.
 *
 * @return A pointer to a User struct.
 */
struct User *user_new(const char *display_name, const char *username, const char *password)
{
  struct User *user = (struct User *)malloc(sizeof(struct User));
  strcpy(user->display_name, display_name);
  strcpy(user->username, username);
  strcpy(user->password, password);
  user->save = save_user;
  user->update = update_user;
  user->remove = delete_user;
  user->to_json = json_user;
  return user;
}

/**
 * It frees the memory allocated for the user object
 *
 * @param user The user object.
 */
void user_free(struct User *user)
{
  free(user);
}

/**
 * It takes a user struct, inserts it into the database, and returns the id of the newly inserted row
 *
 * @param user The user to save
 *
 * @return The return value is the number of rows that were changed by the last SQL statement.
 */
int save_user(struct User *user)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return -1;
  }

  char *sql = "INSERT INTO users (display_name, username, password) VALUES (?, ?, ?)";
  int res = pool->exec(pool, NULL, NULL, sql, 3, user->display_name, user->username, user->password);
  if (res != SQLITE_OK)
  {
    return -1;
  }
  int last_id = sqlite3_last_insert_rowid(pool->db);
  user->id = last_id;
  return 0;
}

/**
 * It takes a user struct, updates it in the database, and returns the number of rows that were changed
 *
 * @param user The user to update
 *
 * @return The return value is the number of rows that were changed by the last SQL statement.
 */
int update_user(struct User *user)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *sql = "UPDATE users SET display_name = ?, username = ?, password = ?, status = ? WHERE id = ?";
  int res = pool->exec(pool, NULL, NULL, sql, 5,
                       user->display_name, user->username, user->password,
                       convert_long_to_string(user->status), convert_long_to_string(user->id));
  if (res != SQLITE_OK)
    return -1;
  return 0;
}

/**
 * It takes a user struct, deletes it from the database, and returns the number of rows that were changed
 *
 * @param user The user to delete
 *
 * @return The return value is the number of rows that were changed by the last SQL statement.
 */
int delete_user(struct User *user)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return -1;

  char *sql = "DELETE FROM users WHERE id = ?";
  int res = pool->exec(pool, NULL, NULL, sql, 1, convert_long_to_string(user->id));
  if (res != SQLITE_OK)
    return -1;
  return 0;
}

/**
 * It takes a user struct, and returns a json string
 *
 * @param user The user to convert to json
 *
 * @return A json string
 */
char *json_user(struct User *user)
{
  char *json = (char *)malloc(1024);
  sprintf(json, "{\"id\": %ld, \"display_name\": \"%s\", \"username\": \"%s\", \"status\": %d}", user->id, user->display_name, user->username, user->status);
  return json;
}

/**
 * It finds a user by their id
 *
 * @param id The id of the user to find
 *
 * @return A pointer to a User struct.
 */
struct User *user_find_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *sql = "SELECT id, display_name, username, password, status FROM users WHERE id = ?";
  struct User *user = NULL;
  int res = pool->exec(pool, _find_user_callback, &user, sql, 1, convert_long_to_string(id));

  if (res != SQLITE_OK)
    return NULL;
  return user;
}

struct User *user_find_by_username(const char *username)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
    return NULL;

  char *sql = "SELECT id, display_name, username, password, status FROM users WHERE username = ?";
  struct User *user = NULL;
  int res = pool->exec(pool, _find_user_callback, &user, sql, 1, username);

  if (res != SQLITE_OK)
    return NULL;
  return user;
}

void _find_user_callback(sqlite3_stmt *res, void *arg)
{
  struct User *user = user_new(
      (char *)sqlite3_column_text(res, 1),  // display_name
      (char *)sqlite3_column_text(res, 2),  // username
      (char *)sqlite3_column_text(res, 3)); // password
  user->id = sqlite3_column_int(res, 0);
  user->status = sqlite3_column_int(res, 4);
  *(struct User **)arg = user;
}
