#include "logger/logger.h"
#include "networking.h"
#include <signal.h>

void sigintHandler()
{
  log_info("Closing server...");
  logger_close();
  exit(0);
}

int main()
{
  signal(SIGINT, sigintHandler);

  int ret;
  if ((ret = logger_init(D_INFO, NULL)) < 0)
    return (ret);

  struct TFTPServer server = tftp_server_constructor(INADDR_ANY, 3069, 1, "upload");
  server.launch(&server);

  return (0);
}
