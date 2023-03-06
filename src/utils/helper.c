
#include "utils/helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

/**
 * It takes an integer and returns a string
 *
 * @param i The integer to convert to a string.
 *
 * @return A pointer to a string.
 */
char *convert_int_to_string(int i)
{
  char *str = (char *)malloc(10);
  sprintf(str, "%d", i);
  return str;
}

/**
 * It takes a float and returns a string
 *
 * @param f The float value to be converted to a string.
 *
 * @return A pointer to a string.
 */
char *convert_float_to_string(float f)
{
  char *str = (char *)malloc(10);
  sprintf(str, "%f", f);
  return str;
}

/**
 * It takes a double and returns a string
 *
 * @param d The double value to convert to a string.
 *
 * @return A pointer to a string.
 */
char *convert_double_to_string(double d)
{
  char *str = (char *)malloc(10);
  sprintf(str, "%lf", d);
  return str;
}

/**
 * It takes a long integer and returns a string representation of that integer
 *
 * @param l The long to convert to a string.
 *
 * @return A pointer to a string.
 */
char *convert_long_to_string(long l)
{
  char *str = (char *)malloc(10);
  sprintf(str, "%ld", l);
  return str;
}

/**
 * It returns the current time in a string format
 *
 * @return A pointer to a string.
 */
char *get_current_time()
{
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  char *time = malloc(20);
  strftime(time, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
  return time;
}

/**
 * It recursively deletes a directory and all of its contents
 * 
 * @param path The path to the directory to be deleted.
 * 
 * @return The return value is the number of bytes that were written to the file.
 */
int remove_directory(const char *path)
{
  DIR *d = opendir(path);
  size_t path_len = strlen(path);
  int r = -1;

  if (d)
  {
    struct dirent *p;

    r = 0;
    while (!r && (p = readdir(d)))
    {
      int r2 = -1;
      char *buf;
      size_t len;

      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
        continue;

      len = path_len + strlen(p->d_name) + 2;
      buf = malloc(len);

      if (buf)
      {
        struct stat statbuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);
        if (!stat(buf, &statbuf))
        {
          if (S_ISDIR(statbuf.st_mode))
            r2 = remove_directory(buf);
          else
            r2 = unlink(buf);
        }
        free(buf);
      }
      r = r2;
    }
    closedir(d);
  }

  if (!r)
    r = rmdir(path);

  return r;
}

/**
 * It creates a directory
 * 
 * @param path The path to the directory you want to create.
 * 
 * @return The return value is the return value of the mkdir function.
 */
int create_directory(const char *path)
{
  return mkdir(path, 0700);
}

/**
 * "Move a file from one location to another."
 * 
 * @param src The source file to move.
 * @param dest The destination file path.
 * 
 * @return The return value is the result of the rename() function.
 */
int move_file(const char *src, const char *dest)
{
  return rename(src, dest);
}