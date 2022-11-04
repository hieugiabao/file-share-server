#include "logger/logger.h"

int logger_init(int level, char *log_file)
{
	int ret;

	g_log_lvl = D_OFF;
	if (level >= D_FATAL && level <= D_TRACE)
	{
		g_log_lvl = level;
		if ((ret = logger_init_open_file(log_file)) < 0)
		{
			fprintf(stdout, "\n\033[32m>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \033[0m");
			fprintf(stdout, "\033[32mNEW INSTANCE OF THE APPLICATION\033[0m");
			fprintf(stdout, "\033[32m <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\033[0m\n\n");
		}
		else
		{
			dprintf(g_log_fd, "\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ");
			dprintf(g_log_fd, "NEW INSTANCE OF THE APPLICATION");
			dprintf(g_log_fd, " <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\n");
		}
	}
	return (0);
}
