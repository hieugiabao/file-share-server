
#ifndef HELPERS_H
#define HELPERS_H

#include <string.h>

char *convert_int_to_string(int);
char *convert_float_to_string(float);
char *convert_double_to_string(double);
char *convert_long_to_string(long);

char *get_current_time();
int remove_directory(const char *path);
int create_directory(const char *path);
int move_file(const char *src, const char *dest);

#endif
