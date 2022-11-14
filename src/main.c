#include "logger/logger.h"
#include "tftp/tftp_server.h"

void sigintHandler()
{
  close_server();
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
  init_server("0.0.0.0", "3069", "upload", 1);
  serve();
  close_server();
  return (0);
}
