
#ifndef _MODEL_GROUP_H_
#define _MODEL_GROUP_H_

#include "user.h"
#include "data_structures/linked_list.h"

/* Group table */
struct Group
{
  /* Public member variables */

  long id;           // Primary key
  char *name;        // Group name
  char *description; // Group description
  char *avatar;      // Group avatar
  long owner_id;     // Foreign key
  int status;        // Group status
  char *code;        // Group code
  char *created_at;  // Group creation date

  /* Private member variables */

  struct User *_owner;         // Group owner
  struct LinkedList *_members; // Group members

  /* Public member functions */

  // Save group to database
  int (*save)(struct Group *group);
  // Update group info
  int (*update)(struct Group *group);
  // Delete group from database
  int (*remove)(struct Group *group);
  // Add user to group members
  int (*add_member)(struct Group *group, struct User *user);
  // Remove user from group members
  int (*remove_member)(struct Group *group, struct User *user);
  // Get group owner
  struct User *(*get_owner)(struct Group *group);
  // Get group members
  struct LinkedList *(*get_members)(struct Group *group);
  // Return group as json
  char *(*to_json)(struct Group *group);
};

struct Group *group_new(char *name, char *description, char *avatar, long owner_id);
void group_free(struct Group *group);

struct Group *group_find_by_id(long id);
struct Group *group_find_by_name(char *name);
struct Group *group_find_by_code(char *code);
struct LinkedList *group_find_by_member(long member_id);
char *group_to_json(struct Group *group);

#endif
