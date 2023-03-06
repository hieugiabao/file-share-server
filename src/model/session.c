#include "model/session.h"
#include "database/db.h"
#include "logger/logger.h"
#include "utils/helper.h"

#include <string.h>

/* Private methods prototype */

int save_session(struct Session *session);
int delete_session(struct Session *session);
struct User *get_user(struct Session *session);
int is_expired(struct Session *session);

void _find_session_callback(sqlite3_stmt *res, void *arg);
void _find_token_callback(sqlite3_stmt *res, void *arg);
time_t time_from_string(char *str);

/* Public methods implements */

/**
 * `session_new` creates a new session object
 *
 * @param user_id The user id of the user who is logged in.
 * @param token A random string that is used to identify the session.
 *
 * @return A pointer to a struct Session
 */
struct Session *session_new(long user_id, char *token)
{
  struct Session *session = (struct Session *)malloc(sizeof(struct Session));
  session->user_id = user_id;
  if (token != NULL)
    session->token = strdup(token);
  session->save = save_session;
  session->_user = NULL;
  session->delete_session = delete_session;
  session->get_user = get_user;
  session->is_expired = is_expired;
  return session;
}

/**
 * It frees the memory allocated for the session object
 *
 * @param session The session object.
 */
void session_free(struct Session *session)
{
  if (session->_user != NULL)
    user_free(session->_user);
  free(session->token);
  free(session);
}

/**
 * It takes a session struct, inserts it into the database, and returns the id of the newly inserted row
 *
 * @param session The session to save
 *
 * @return The return value is the number of rows that were changed by the last SQL statement.
 */
int save_session(struct Session *session)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return -1;
  }

  char *sql = "INSERT INTO sessions (user_id) VALUES (?)";
  int res = pool->exec(pool, NULL, NULL, sql, 1, convert_long_to_string(session->user_id));
  if (res != SQLITE_OK)
  {
    return -1;
  }
  int last_id = sqlite3_last_insert_rowid(pool->db);
  session->id = last_id;

  // retrive token from table
  sql = "SELECT token FROM sessions WHERE id = ?";
  char *token = NULL;
  res = pool->exec(pool, _find_token_callback, &token, sql, 1, convert_long_to_string(session->id));

  if (res != SQLITE_OK)
  {
    return -1;
  }
  session->token = token;

  return 0;
}

/**
 * It takes a session struct, deletes it from the database, and returns the number of rows deleted
 *
 * @param session The session to delete
 *
 * @return The return value is the number of rows that were changed by the last SQL statement.
 */
int delete_session(struct Session *session)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return -1;
  }

  char *sql = "DELETE FROM sessions WHERE id = ?";
  int res = pool->exec(pool, NULL, NULL, sql, 1, convert_long_to_string(session->id));
  if (res != SQLITE_OK)
  {
    return -1;
  }
  return res;
}

/**
 * It takes a session struct, and returns the user object
 *
 * @param session The session to get user
 *
 * @return The return value is the user object.
 */
struct User *get_user(struct Session *session)
{
  if (session->_user != NULL)
    return session->_user;
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return NULL;
  }

  struct User *user = user_find_by_id(session->user_id);

  session->_user = user;
  return user;
}

/**
 * If the session was created more than 30 days ago, it's expired
 *
 * @param session The session object
 *
 * @return The function is_expired() returns 1 if the session has been created more than 30 days ago, and 0 otherwise.
 */
int is_expired(struct Session *session)
{
  if (session->created_at == NULL)
    return 0;
  time_t now = time(NULL);
  time_t created_at = time_from_string(session->created_at);
  double seconds = difftime(now, created_at);
  if (seconds > 30 * 24 * 60 && 0 > 1)
    return 1;
  else
    return 0;
}

/**
 * It finds a session by its id
 *
 * @param id The id of the session to find
 *
 * @return A session object
 */
struct Session *session_find_by_id(long id)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return NULL;
  }

  char *sql = "SELECT id, user_id, token, created_at FROM sessions WHERE id = ?";
  struct Session *session = NULL;
  int res = pool->exec(pool, _find_session_callback, &session, sql, 1, convert_long_to_string(id));
  if (res != SQLITE_OK)
  {
    return NULL;
  }
  return session;
}

/**
 * It finds a session by token
 *
 * @param token The token to search for
 *
 * @return A session object
 */
struct Session *session_find_by_token(char *token)
{
  struct DatabaseManager *manager = get_db_manager();
  struct DatabasePool *pool = manager->get_pool(manager, NULL);
  if (pool == NULL)
  {
    log_error("Cannot get database pool");
    return NULL;
  }

  char *sql = "SELECT id, user_id, token, created_at FROM sessions WHERE token = ?";
  struct Session *session = NULL;
  int res = pool->exec(pool, _find_session_callback, &session, sql, 1, token);
  if (res != SQLITE_OK)
  {
    return NULL;
  }
  return session;
}

/**
 * It takes a SQLite result set and a pointer to a Session pointer, and it sets the Session pointer to a new Session object
 * with the data from the result set
 *
 * @param res The result of the query.
 * @param arg a pointer to a pointer to a Session struct.
 */
void _find_session_callback(sqlite3_stmt *res, void *arg)
{
  struct Session *session = session_new(
      sqlite3_column_int(res, 1),           // user_id
      (char *)sqlite3_column_text(res, 2)); // token
  session->id = sqlite3_column_int(res, 0);
  session->created_at = strdup((char *)sqlite3_column_text(res, 3));
  *(struct Session **)arg = session;
}

void _find_token_callback(sqlite3_stmt *res, void *arg)
{
  *(char **)arg = strdup((char *)sqlite3_column_text(res, 0));
}

/**
 * It takes a string in the format "YYYY-MM-DD HH:MM:SS" and returns a time_t value
 *
 * @param str The string to convert to a time_t.
 *
 * @return The time in seconds since the Epoch.
 */
time_t time_from_string(char *str)
{
  struct tm tm;
  strptime(str, "%Y-%m-%d %H:%M:%S", &tm);
  return mktime(&tm);
}
