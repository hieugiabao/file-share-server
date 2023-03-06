
#include "http/controller/user_controller.h"
#include "model/user.h"
#include "model/session.h"
#include "http/helper/helper.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * It takes a username and password from the query string, finds the user in the database, and if the password matches,
 * returns a 200 OK
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return a pointer to a string.
 */
char *login(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *username = request->body.search(&request->body, "username", 9);
  char *password = request->body.search(&request->body, "password", 9);

  if (username == NULL || password == NULL)
  {
    return format_422();
  }

  struct User *user = user_find_by_username(username);
  if (user == NULL)
  {
    return format_401();
  }

  if (strcmp(user->password, password) != 0)
  {
    return format_401();
  }

#define TOKEN_LENGTH 64
  struct Session *session = session_new(user->id, NULL);
  session->save(session);

  char *response = malloc(100);
  size_t length = 0;
  char *encoded_token = base64_encode((unsigned char *)session->token, TOKEN_LENGTH, &length);
  sprintf(response, "{\"token\": \"%.*s\"}", (int)length, encoded_token);

  session_free(session);
  user_free(user);
  return format_200_with_content_type(response, "application/json");
}

/**
 * It takes the username, password, and display name from the query string, checks if the username is already taken, and if
 * not, creates a new user with the given information
 *
 * @param server The server object.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *register_user(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  char *username = request->body.search(&request->body, "username", 9);
  char *password = request->body.search(&request->body, "password", 9);
  char *display_name = request->body.search(&request->body, "display_name", 13);

  if (username == NULL || password == NULL || display_name == NULL)
  {
    return format_422();
  }

  struct User *user = user_find_by_username(username);
  if (user != NULL)
  {
    return format_409();
  }

  user = user_new(display_name, username, password);
  if (user == NULL)
  {
    return format_500();
  }

  if (user->save(user) != 0)
  {
    return format_500();
  }
  user_free(user);
  return format_200();
}

/**
 * It gets the user from the request, and if it exists, it returns a 200 response with the user's information in JSON
 * format
 *
 * @param server The server object that is handling the request.
 * @param request The HTTPRequest object that contains the request information.
 *
 * @return A pointer to a string.
 */
char *get_user_info(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;
  struct User *user = get_user_from_request(request, NULL);
  if (user == NULL)
  {
    return format_401();
  }

  char *response = user->to_json(user);
  user_free(user);
  return format_200_with_content_type(response, "application/json");
}

char *logout(struct HTTPServer *server, struct HTTPRequest *request)
{
  (void)server;

  char token[100];

  struct User *user = get_user_from_request(request, token);
  if (user == NULL)
  {
    return format_401();
  }

  struct Session *session = session_find_by_token(token);
  if (session == NULL)
  {
    return format_401();
  }

  if (session->delete_session(session) != 0)
  {
    return format_500();
  }

  session_free(session);
  user_free(user);
  return format_200();
}
