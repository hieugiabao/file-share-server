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

  struct HTTPServer server = http_server_constructor(INADDR_ANY, 8000);
  server.launch(&server);

  return (0);
}
