
#ifndef DATABASE_H
#define DATABASE_H

#include "vendor/sqlite3/sqlite3.h"
#include "data_structures/dictionary.h"

/**
 * Database pool
 */
struct DatabasePool
{
  /* Public member variables */

  char *path;        // database path
  char *name;        // database name
  sqlite3 *db;       // database connection

  /* Public methods */

  // open connection to database
  int (*open)(struct DatabasePool *pool);
  // close connection to database
  int (*close)(struct DatabasePool *pool);
  // execute query
  int (*exec)(struct DatabasePool *pool, void (*callback)(sqlite3_stmt *res, void *arg), void *arg, const char *sql, int num, ...);
};

/**
 * Database manager
 */
struct DatabaseManager
{
  /* Public member variables */

  struct Dictionary pools; // database pools

  /* Public methods */

  // get database pool
  struct DatabasePool *(*get_pool)(struct DatabaseManager *manager, char *name);
  // add database pool
  int (*add_pool)(struct DatabaseManager *manager, char *name, struct DatabasePool *pool);
};

int connect_db(const char *uri, char *name);

struct DatabaseManager *get_db_manager();
void database_manager_destructor(struct DatabaseManager *manager);

#endif
