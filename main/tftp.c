#include "logger/logger.h"
#include "networking/tftp.h"
#include "setting.h"

#include <signal.h>
#include <setjmp.h>

static jmp_buf env;

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

  struct TFTPServer tftp_server;

  if (!setjmp(env))
  {
    int ret;
    if ((ret = logger_init(D_DEBUG, NULL)) < 0)
      return (ret);

    tftp_server = tftp_server_constructor(INADDR_ANY, 8069, 1, UPLOAD_DIR);
    tftp_server.launch(&tftp_server);
  }
  else
  {
    tftp_server_destructor(&tftp_server);
    log_info("Closing server...");
    logger_close();
    exit(0);
  }

  return (0);
}
