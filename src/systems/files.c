#include "systems/files.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * It opens a file, seeks to the end of the file, gets the position of the file pointer, closes the file, and returns the
 * position of the file pointer
 *
 * @param path The path to the file you want to get the size of.
 *
 * @return The size of the file in bytes.
 */
unsigned long get_file_size(char *path)
{
  FILE *file = fopen(path, "r");
  fseek(file, 0, SEEK_END);
  unsigned long size = ftell(file);
  fclose(file);
  return size;
}

/**
 * It opens a file, seeks to the end of the file, gets the position of the file pointer, then seeks back to the beginning
 * of the file
 *
 * @param file The file to get the size of.
 *
 * @return The size of the file.
 */
unsigned long get_file_size_internal(FILE *file)
{
  fseek(file, 0, SEEK_END);
  unsigned long size = ftell(file);
  fseek(file, 0, SEEK_SET);
  return size;
}

/**
 * It writes a file
 *
 * @param path The path to the file to write to.
 * @param data The data to be written to the file.
 * @param size The size of the file in bytes.
 */
void write_file(char *path, void *data, unsigned long size)
{
  FILE *file = fopen(path, "w");
  fwrite(data, size, 1, file);
  fclose(file);
}

/**
 * It opens a file, appends data to it, and closes it
 *
 * @param path The path to the file you want to append to.
 * @param data The data to be written to the file.
 * @param size The size of the data to be written.
 */
void append_file(char *path, void *data, unsigned long size)
{
  FILE *file = fopen(path, "a");
  fwrite(data, size, 1, file);
  fclose(file);
}

/**
 * It reads a file into memory
 *
 * @param path The path to the file to read.
 *
 * @return A pointer to the data read from the file.
 */
void *read_file(char *path)
{
  FILE *file = fopen(path, "r");
  unsigned long size = get_file_size_internal(file);
  void *data = malloc(size);
  fread(data, size, 1, file);
  return data;
}
