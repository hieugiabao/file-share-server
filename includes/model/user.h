
#ifndef _MODEL_USER_H_
#define _MODEL_USER_H_

/**
 * User status
 */
enum UserStatus
{
  USER_STATUS_ACTIVE = 1,   // default
  USER_STATUS_INACTIVE = 2, // not used
  USER_STATUS_DELETED = 3   // deleted
};

/**
 * User model
 */
struct User
{
  /* Public fields */

  long id;                // primary key
  char display_name[255]; // full name
  char username[50];      // login name
  char password[50];      // password
  enum UserStatus status; // status

  /* Public methods */

  // save user to database
  int (*save)(struct User *user);
  // update user to database
  int (*update)(struct User *user);
  // delete user from database
  int (*remove)(struct User *user);
  // convert user to json format
  char *(*to_json)(struct User *user);
};

/* Public method */

struct User *user_new(const char *display_name, const char *username, const char *password);
void user_free(struct User *user);

struct User *user_find_by_id(long id);
struct User *user_find_by_username(const char *username);

#endif
