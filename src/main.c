#include "logger/logger.h"
#include "networking.h"
#include "systems/thread_pool.h"
#include "database/db.h"

#include <signal.h>
#include <setjmp.h>

#define UPLOAD_DIR "./upload"
#define DATABASE_URI "test.db"

static jmp_buf env;

void *tftp_init_handler(void *arg);
void *http_init_handler(void *arg);

void sqliteversion_callback(sqlite3_stmt *res, void *arg)
{
  (void)arg;

  int i;
  for (i = 0; i < sqlite3_column_count(res); i++)
  {
    log_info("%s - %s", sqlite3_column_name(res, i), sqlite3_column_text(res, i));
  }
}

void sigintHandler()
{
  char chose[10];
  printf("Do you want to close server? [y/N] ");
  scanf("%s", chose);
  if (strcmp(chose, "y") == 0)
  {
    longjmp(env, 1);
  }
}

int main()
{
  signal(SIGINT, sigintHandler);

  struct HTTPServer http_server;
  struct TFTPServer tftp_server;

  struct ThreadPool *pool = thread_pool_constructor(2);
  struct DatabaseManager *manager = get_manager();

  if (!setjmp(env))
  {
    int ret;
    if ((ret = logger_init(D_INFO, NULL)) < 0)
      return (ret);
    if (connect_db(DATABASE_URI, NULL) == -1)
    {
      log_error("Can't connect to database");
    }

    struct DatabasePool *db_pool = manager->get_pool(manager, NULL);

    db_pool->exec(db_pool, sqliteversion_callback, NULL, "SELECT SQLITE_VERSION() as sqlite_version", 0);

    struct ThreadJob create_http_server_job = thread_job_constructor(http_init_handler, &http_server);
    pool->add_work(pool, create_http_server_job);

    struct ThreadJob create_tftp_server_job = thread_job_constructor(tftp_init_handler, &tftp_server);
    pool->add_work(pool, create_tftp_server_job);

    pool->wait_all(pool);
  }
  else
  {
    database_manager_destructor(manager);
    tftp_server_destructor(&tftp_server);
    http_server_destructor(&http_server);
    thread_pool_destructor(pool);
    log_info("Closing server...");
    logger_close();
    exit(0);
  }

  return (0);
}

void *tftp_init_handler(void *arg)
{
  struct TFTPServer *tftp_server = (struct TFTPServer *)arg;
  *tftp_server = tftp_server_constructor(INADDR_ANY, 8069, 1, UPLOAD_DIR);
  tftp_server->launch(tftp_server);

  return NULL;
}

void *http_init_handler(void *arg)
{
  struct HTTPServer *http_server = (struct HTTPServer *)arg;
  *http_server = http_server_constructor(INADDR_ANY, 8000);
  http_server->launch(http_server);

  return NULL;
}
