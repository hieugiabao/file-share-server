#include "logger/logger.h"

int g_log_fd;
int g_log_lvl;

int	logger_init_open_file(char *log_file)
{
	if (log_file == NULL)
		g_log_fd = -1;
	g_log_fd = open(log_file, O_WRONLY | O_APPEND | O_CREAT, 0755);
	return (g_log_fd);
}
