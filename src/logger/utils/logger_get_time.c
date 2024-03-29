#include "logger/logger.h"

char *logger_get_time(void)
{
  time_t tm;
  char *buf;
  struct tm *tm_info;

  if (!(buf = (char *)malloc(sizeof(char) * 9)))
    return (NULL);
  tm = time(NULL);
  tm_info = localtime(&tm);
  strftime(buf, 23, "%Y-%m-%d %H:%M:%S", tm_info);
  return (buf);
}
