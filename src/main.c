#include "logger/logger.h"

int main()
{
  int ret;
  if ((ret = logger_init(D_INFO, "log/app.log")) < 0)
    return (ret);
  log_fatal("%s", "fatal message");
	log_error("%s", "error message");
	log_warn("%s", "warning message");
	log_success("%s", "success message");
	log_info("%s", "info message");
	log_debug("%s", "debug message");
	log_trace("%s", "trace message");
  logger_close();
  return (0);
}
