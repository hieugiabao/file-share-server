

#ifndef _FILES_H_
#define _FILES_H_

unsigned long get_file_size(char *path);
void write_file(char *path, void *data, unsigned long size);
void append_file(char *path, void *data, unsigned long size);
void *read_file(char *path);

#endif // _FILES_H_
