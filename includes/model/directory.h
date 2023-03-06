
#ifndef _MODEL_DIRECTORY_H_
#define _MODEL_DIRECTORY_H_

#include "user.h"
#include "group.h"
#include "model.h"
#include "file.h"
#include "data_structures/linked_list.h"

struct FNode
{
  union
  {
    struct Directory *directory;
    struct File *file;
  } node;
  enum FType type;
};

/// @brief  Directory table
struct Directory
{
  /* Public member variables */

  long id;                    // directory id
  char *name;                 // directory name
  enum Permission permission; // permission
  char *path;                 // path
  long parent_id;             // parent directory id
  long group_id;              // group id
  long owner_id;              // owner id
  char *created_at;           // created at
  char *updated_at;           // updated at

  /* Private member variables */

  struct User *_owner;          // owner
  struct Group *_group;         // group
  struct Directory *_parent;    // parent directory
  struct LinkedList *_children; // children node

  /* Public member functions */

  int (*save)(struct Directory *);                        // save directory
  int (*remove)(struct Directory *);                      // remove directory
  int (*update)(struct Directory *);                      // update directory
  struct User *(*get_owner)(struct Directory *);          // get owner
  struct Group *(*get_group)(struct Directory *);         // get group
  struct Directory *(*get_parent)(struct Directory *);    // get parent directory
  struct LinkedList *(*get_children)(struct Directory *); // get children
  char *(*to_json)(struct Directory *);                   // convert to json
};

/* Public method */

struct Directory *directory_new(const char *name, long user_id, long group_id, long *parent_id);
void directory_free(struct Directory *directory);
struct Directory *directory_find_by_id(long id);
struct LinkedList *get_root_node_by_group(long group_id);

#endif
