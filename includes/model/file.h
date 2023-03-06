
#ifndef _MODEL_FILE_H_
#define _MODEL_FILE_H_

#include "user.h"
#include "group.h"
#include "model.h"
#include "directory.h"

/* File table */
struct File
{
  /* Public member variables */

  long id;                    // file id
  char *name;                 // file name
  long size;                  // file size
  enum Permission permission; // file permission
  char *path;                 // file path
  long directory_id;          // directory id
  long group_id;              // group id
  long owner_id;              // owner id
  long modified_by;          // modified by
  char *created_at;           // created at
  char *updated_at;           // updated at

  /* Private member variables */

  struct User *_owner;          // file owner
  struct User *_modified_by;    // file modified by
  struct Group *_group;         // file group
  struct Directory *_directory; // file directory

  /* Public member functions */

  int (*save)(struct File *);                        // save file
  int (*remove)(struct File *);                      // remove file
  int (*update)(struct File *);                      // update file
  struct User *(*get_owner)(struct File *);          // get file owner
  struct User *(*get_modified_user)(struct File *);  // get file modified by
  struct Group *(*get_group)(struct File *);         // get file group
  struct Directory *(*get_directory)(struct File *); // get file directory
  char *(*to_json)(struct File *);                   // convert file to json
};

struct File *file_new(char *fullname, long size, long user_id, long group_id, long *directory_id);
void file_free(struct File *file);
struct File *file_find_by_id(long id);
char *file_to_json(struct File *file);

#endif