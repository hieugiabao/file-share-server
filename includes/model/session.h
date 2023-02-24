
#ifndef _MODEL_SESSION_H_
#define _MODEL_SESSION_H_

#include "user.h"

/* Session table */
struct Session
{
  /* Public member variables */

  long id;          // primary key
  long user_id;     // foreign key
  char *token;      // session token
  char *created_at; // creation date

  /* Private member variables */

  struct User *_user; // user object

  /* Public member functions */

  // Save session to database
  int (*save)(struct Session *session);
  // Delete session from database
  int (*delete_session)(struct Session *session);
  // Get user from session
  struct User *(*get_user)(struct Session *session);
  // Check if session is expired
  int (*is_expired)(struct Session *session);
};

/* Session table functions */

struct Session *session_new(long user_id, char *token);
void session_free(struct Session *session);

struct Session *session_find_by_id(long id);
struct Session *session_find_by_token(char *token);

#endif
