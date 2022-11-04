#include "logger/logger.h"

int	logger_close(void)
{
  if (g_log_fd == -1)
  {
    fprintf(stdout, "\n\033[32m>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \033[0m");
    fprintf(stdout, "\033[32mEND OF THE APPLICATION INSTANCE\033[0m");
    fprintf(stdout, "\033[32m <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\033[0m\n\n");
    return 0;
  }
  else
  {
    dprintf(g_log_fd, "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ");
    dprintf(g_log_fd, "END OF THE APPLICATION INSTANCE");
    dprintf(g_log_fd, " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
    return (close(g_log_fd));
  }
}
