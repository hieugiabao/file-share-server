
#include "utils/helper.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  char *s = (char *)malloc(64);
  strftime(s, sizeof(s), "%c", tm);
  return s;
}