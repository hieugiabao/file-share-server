#include "logger/logger.h"
#include "networking.h"
#include "systems/thread_pool.h"
#include "database/db.h"
#include "model/user.h"
#include "http/controller/controller.h"
#include "setting.h"

#include <signal.h>
#include <setjmp.h>

static jmp_buf env;

void *tftp_init_handler(void *arg);
void *http_init_handler(void *arg);
void init_database(struct DatabasePool *pool);

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
  // struct TFTPServer tftp_server;

  struct ThreadPool *pool = thread_pool_constructor(1);
  struct DatabaseManager *manager = get_db_manager();

  if (!setjmp(env))
  {
    int ret;
    if ((ret = logger_init(D_DEBUG, NULL)) < 0)
      return (ret);
    if (connect_db(DATABASE_URI, NULL) == -1)
    {
      log_error("Can't connect to database");
    }

    struct DatabasePool *db_pool = manager->get_pool(manager, NULL);

    db_pool->exec(db_pool, sqliteversion_callback, NULL, "SELECT SQLITE_VERSION() as sqlite_version", 0);
    init_database(db_pool);

    struct ThreadJob create_http_server_job = thread_job_constructor(http_init_handler, &http_server);
    pool->add_work(pool, create_http_server_job);

    // struct ThreadJob create_tftp_server_job = thread_job_constructor(tftp_init_handler, &tftp_server);
    // pool->add_work(pool, create_tftp_server_job);

    pool->wait_all(pool);
  }
  else
  {
    database_manager_destructor(manager);
    // tftp_server_destructor(&tftp_server);
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
   
  /* Register routes */

  // auth
  http_server->register_routes(http_server, login, "/login", 1, POST);
  http_server->register_routes(http_server, register_user, "/register", 1, POST);
  http_server->register_routes(http_server, get_me_info, "/me/info", 1, GET);
  http_server->register_routes(http_server, get_user_info, "/user/info", 1, GET);
  http_server->register_routes(http_server, get_my_groups, "/me/group", 1, GET);
  http_server->register_routes(http_server, logout, "/logout", 1, POST);

  // group
  http_server->register_routes(http_server, create_group, "/group/create", 1, POST);
  http_server->register_routes(http_server, update_group, "/group/update", 1, PUT);
  http_server->register_routes(http_server, get_group_members, "/group/members", 1, GET);
  http_server->register_routes(http_server, delete_group, "/group/delete", 1, DELETE);
  http_server->register_routes(http_server, join_group, "/group/join", 1, POST);
  http_server->register_routes(http_server, leave_group, "/group/leave", 1, POST);
  http_server->register_routes(http_server, kick_member_group, "/group/kick", 1, POST);
  http_server->register_routes(http_server, get_group_node_tree, "/group/node", 1, GET);

  http_server->register_routes(http_server, make_directory, "/directory/create", 1, POST);
  http_server->register_routes(http_server, delete_directory, "/directory/delete", 1, DELETE);
  http_server->register_routes(http_server, update_directory, "/directory/update", 1, PUT);
  http_server->register_routes(http_server, get_directory_children, "/directory/getchild", 1, GET);
  http_server->register_routes(http_server, get_directory_info, "/directory/info", 1, GET);

  http_server->register_routes(http_server, create_file, "/file/create", 1, POST);
  http_server->register_routes(http_server, save_file, "/file/save", 1, POST);
  http_server->register_routes(http_server, delete_file, "/file/delete", 1, DELETE);
  http_server->register_routes(http_server, update_file, "/file/update", 1, PUT);
  http_server->register_routes(http_server, get_file, "/file/info", 1, GET);

  http_server->launch(http_server);

  return NULL;
}

void init_database(struct DatabasePool *pool)
{
  int fd = open(DATABASE_INIT_FILE, O_RDONLY);
  if (fd == -1)
  {
    log_error("Can't open file %s", DATABASE_INIT_FILE);
    return;
  }
  // get num of characters
  int num = lseek(fd, 0, SEEK_END);
  // reset file pointer
  lseek(fd, 0, SEEK_SET);

  // read file
  char *buffer = malloc(num);
  int ret = read(fd, buffer, num);

  if (ret == -1)
  {
    log_error("Can't read file %s", DATABASE_INIT_FILE);
    return;
  }

  // slipt string by ;
  char *token = strtok(buffer, ";");
  while (token != NULL)
  {
    // add ; to end of string
    token[strlen(token)] = ';';
    pool->exec(pool, NULL, NULL, token, 0);
    token = strtok(NULL, ";");
  }
  free(buffer);
  close(fd);
}
