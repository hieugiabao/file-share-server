#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "database/db.h"
#include "logger/logger.h"

/* Public variables */
static struct DatabaseManager *manager = NULL;

/* Private helper method prototype */

int open_connect(struct DatabasePool *pool);
int close_connect(struct DatabasePool *pool);
int exec(struct DatabasePool *pool, void (*callback)(sqlite3_stmt *res, void *arg), void *arg, const char *sql, int num, ...);

struct DatabasePool *
database_pool_constructor(const char *uri, char *name);
struct DatabaseManager *database_manager_constructor();
void database_pool_destructor(struct DatabasePool *pool);

size_t _get_key_size(void *key);
void _db_manager_des_callback(void *key, void *value, void *arg);

struct DatabasePool *get_pool(struct DatabaseManager *manager, char *name);
int add_pool(struct DatabaseManager *manager, char *name, struct DatabasePool *pool);

/* Public methods implements */

/**
 * It creates a new database pool object, sets the path to the database, and sets the function pointers to the functions
 * that will be used to open, close, and execute queries on the database
 *
 * @param uri The path to the database file.
 *
 * @return A pointer to a struct DatabasePool.
 */
struct DatabasePool *database_pool_constructor(const char *uri, char *name)
{
  struct DatabasePool *pool = (struct DatabasePool *)malloc(sizeof(struct DatabasePool));
  pool->path = strdup(uri);
  pool->name = strdup(name);
  pool->open = open_connect;
  pool->close = close_connect;
  pool->exec = exec;
  pool->db = NULL;

  return (pool);
}

/**
 * It closes the database connection and frees the memory allocated for the database pool
 *
 * @param pool The database pool object.
 */
void database_pool_destructor(struct DatabasePool *pool)
{
  if (pool->db != NULL)
    pool->close(pool);

  free(pool->name);
  free(pool->path);
}

/**
 * It creates a new database manager object and sets the function pointers to the functions that will be used to get,
 * add, and remove database pools from the manager
 *
 * @return A pointer to a struct DatabaseManager.
 */
struct DatabaseManager *database_manager_constructor()
{
  struct DatabaseManager *manager = (struct DatabaseManager *)malloc(sizeof(struct DatabaseManager));
  manager->pools = dictionary_constructor(compare_string_keys);
  manager->get_pool = get_pool;
  manager->add_pool = add_pool;

  return (manager);
}

void _db_manager_des_callback(void *key, void *value, void *arg)
{
  (void)key;
  (void)arg;
  database_pool_destructor((struct DatabasePool *)value);
}

size_t _get_key_size(void *key)
{
  return sizeof(char[strlen((char *)key)]);
}

/**
 * It frees the memory allocated for the database manager
 *
 * @param manager The database manager object.
 */
void database_manager_destructor(struct DatabaseManager *manager)
{
  // close all connection
  manager->pools.iterate(&manager->pools, _get_key_size, _db_manager_des_callback, NULL);
  dictionary_destructor(&manager->pools, NULL, NULL);
  free(manager);
}

/**
 * It returns the database manager object. If the database manager object is not created, it creates a new one.
 *
 * @return A pointer to a struct DatabaseManager.
 */
struct DatabaseManager *get_db_manager()
{
  if (manager == NULL)
    manager = database_manager_constructor();
  return (manager);
}

/* Private helper methods implements */

/**
 * It opens a connection to the database and sets the database connection to the database pool object
 *
 * @param pool The database pool object.
 *
 * @return 0 if the connection is successful, otherwise it returns a negative number.
 */
int open_connect(struct DatabasePool *pool)
{
  int ret = sqlite3_open(pool->path, &pool->db);
  if (ret != SQLITE_OK)
  {
    log_error("Can't open database: %s", sqlite3_errmsg(pool->db));
    sqlite3_close(pool->db);
    return (-1);
  }
  log_info("Database connection is opened successfully: %s", pool->path);
  return (0);
}

/**
 * It closes the database connection
 *
 * @param pool The database pool object.
 *
 * @return 0 if the connection is closed successfully, otherwise it returns a negative number.
 */
int close_connect(struct DatabasePool *pool)
{
  int ret = sqlite3_close(pool->db);
  if (ret != SQLITE_OK)
  {
    log_error("Can't close database: %s", sqlite3_errmsg(pool->db));
    return (-1);
  }
  return (0);
}

/**
 * It executes a query on the database and calls the callback function with the result of the query
 *
 * @param pool The database pool object.
 * @param callback The callback function that will be called with the result of the query.
 * @param arg The argument that will be passed to the callback function.
 * @param sql The query that will be executed on the database.
 * @param num The number of arguments that will be passed to the query.
 * @param ... The arguments that will be passed to the query.
 *
 * @return 0 if the query is executed successfully, otherwise it returns a negative number.
 */
int exec(struct DatabasePool *pool, void (*callback)(sqlite3_stmt *res, void *arg), void *arg, const char *sql, int num, ...)
{
  va_list args;
  va_start(args, num);
  sqlite3_stmt *res;
  int ret = sqlite3_prepare_v2(pool->db, sql, -1, &res, NULL);
  if (ret != SQLITE_OK)
  {
    log_error("Can't prepare statement: %s", sqlite3_errmsg(pool->db));
    return (-1);
  }
  for (int i = 0; i < num; i++)
  {
    char *value = va_arg(args, char *);
    sqlite3_bind_text(res, i + 1, value, -1, SQLITE_STATIC);
  }
  va_end(args);

  log_debug("Query: %s", sqlite3_expanded_sql(res));

  while ((ret = sqlite3_step(res)) == SQLITE_ROW && callback != NULL)
  {
    callback(res, arg);
  }

  if (ret != SQLITE_DONE)
  {
    log_error("Query error: %s", sqlite3_errmsg(pool->db));
    sqlite3_finalize(res);
    return ret;
  }

  sqlite3_finalize(res);
  return (SQLITE_OK);
}

/**
 * It returns the database pool object with the given name
 *
 * @param manager The database manager object.
 * @param name The name of the database pool.
 *
 * @return A pointer to a struct DatabasePool.
 */
struct DatabasePool *get_pool(struct DatabaseManager *manager, char *name)
{
  if (name == NULL)
    name = "default";
  return *(struct DatabasePool **)(manager->pools.search(&manager->pools, name, strlen(name)));
}

/**
 * It adds a database pool object to the database manager object
 *
 * @param manager The database manager object.
 * @param name The name of the database pool.
 * @param pool The database pool object.
 */
int add_pool(struct DatabaseManager *manager, char *name, struct DatabasePool *pool)
{
  if (manager->pools.search(&manager->pools, name, strlen(name)) != NULL)
  {
    log_error("Database pool with name %s already exists", name);
    return (-1);
  }
  manager->pools.insert(&manager->pools, name, strlen(name), &pool, sizeof(struct DatabasePool **));
  return (0);
}

/* Public methods implements */

/**
 * It creates a new database pool, adds it to the database manager, and then opens the pool
 *
 * @param uri The connection string to the database.
 * @param name The name of the connection. This is used to identify the connection.
 *
 * @return The return value is the number of connections that were successfully opened.
 */
int connect_db(const char *uri, char *name)
{
  struct DatabaseManager *manager = get_db_manager();
  if (name == NULL)
    name = "default";
  struct DatabasePool *pool = database_pool_constructor(uri, name);
  if (manager->add_pool(manager, name, pool) != 0)
  {
    database_pool_destructor(pool);
    return (-1);
  }

  return pool->open(pool);
}
