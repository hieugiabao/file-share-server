#include "logger/logger.h"
#include "networking.h"
#include "systems/thread_pool.h"
#include <signal.h>
#include <setjmp.h>

#define UPLOAD_DIR "./upload"

static jmp_buf env;

void *tftp_init_handler(void *arg);
void *http_init_handler(void *arg);

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

  if (!setjmp(env))
  {
    int ret;
    if ((ret = logger_init(D_INFO, NULL)) < 0)
      return (ret);

    struct ThreadJob create_http_server_job = thread_job_constructor(http_init_handler, &http_server);
    pool->add_work(pool, create_http_server_job);

    struct ThreadJob create_tftp_server_job = thread_job_constructor(tftp_init_handler, &tftp_server);
    pool->add_work(pool, create_tftp_server_job);

    pool->wait_all(pool);
  }
  else
  {
    // thread_pool_destructor(pool);
    tftp_server_destructor(&tftp_server);
    http_server_destructor(&http_server);
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
