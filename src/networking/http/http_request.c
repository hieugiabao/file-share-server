
#include "networking/http/http_request.h"
#include "data_structures/queue.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Private member methods prototypes */

void extract_request_line_fields(struct HTTPRequest *request, char *request_line);
void extract_request_query(struct HTTPRequest *request, char *query_string);
void extract_header_fields(struct HTTPRequest *request, char *header_fields);
void extract_body(struct HTTPRequest *request, char *body);

/* Constructor */

/**
 * Creates an initialized instance of an HTTPRequest using a properly formatted string.
 *
 * @param request_string The HTTP request string
 *
 * @return A struct HTTPRequest
 */
struct HTTPRequest http_request_constructor(char *request_string)
{
  struct HTTPRequest request;
  // Convert the request string literal and replace the blank line with a vertical |
  char requested[strlen(request_string)];
  strcpy(requested, request_string);

  for (size_t i = 0; i < strlen(requested) - 2; i++)
  {
    if (requested[i] == '\r' && requested[i + 1] == '\n' && requested[i + 2] == '\r' && requested[i + 3] == '\n')
    {
      requested[i + 3] = '|';
      i += 3;
    }
  }
  // Separate the request string into its components
  char *request_line = strtok(requested, "\n");
  char *header_fields = strtok(NULL, "|");
  char *body = strtok(NULL, "\0");

  // Parse each section as needed
  extract_request_line_fields(&request, request_line);
  extract_header_fields(&request, header_fields);
  extract_body(&request, body);

  return request;
}

/**
 * It takes a pointer to a `struct HTTPRequest` and frees all the memory associated with it
 *
 * @param request The HTTPRequest struct to be destructed.
 */
void http_request_destructor(struct HTTPRequest *request)
{
  dictionary_destructor(&request->request_line, NULL, NULL);
  dictionary_destructor(&request->header_fields, NULL, NULL);
  dictionary_destructor(&request->body, NULL, NULL);
  dictionary_destructor(&request->query, NULL, NULL);
}

/* Private member methods implementation */

/**
 * Parses out the request line to retrieve the method, uri, and http version.
 *
 * @param request The HTTPRequest object that we're going to insert the request line fields into.
 * @param request_line The request line from the HTTP request.
 */
void extract_request_line_fields(struct HTTPRequest *request, char *request_line)
{
  char fields[strlen(request_line)];
  strcpy(fields, request_line);
  char *method = strtok(fields, " ");
  char *path = strtok(NULL, " ");
  char *http_version = strtok(NULL, "\0");
  // Insert the results into the request object as a dictionary.
  struct Dictionary request_line_dict = dictionary_constructor(compare_string_keys);
  request_line_dict.insert(&request_line_dict, "method", sizeof("method"), method, sizeof(char[strlen(method)]));
  char *uri = strtok(path, "?");
  char *query_string = strtok(NULL, "\0");
  extract_request_query(request, query_string);

  request_line_dict.insert(&request_line_dict, "uri", sizeof("uri"), uri, sizeof(char[strlen(uri)]));
  request_line_dict.insert(&request_line_dict, "http_version", sizeof("http_version"), http_version, sizeof(char[strlen(http_version)]));

  request->request_line = request_line_dict;
}

/**
 * Parses out the header fields.
 *
 * @param request A pointer to the HTTPRequest structure that you need to fill in.
 * @param header_fields The header fields string, which is the part of the request after the first line and before the
 * blank line.
 */
void extract_header_fields(struct HTTPRequest *request, char *header_fields)
{
  char fields[strlen(header_fields)];
  strcpy(fields, header_fields);

  // Save each line of the input into a queue
  struct Queue headers = queue_constructor();
  char *field = strtok(fields, "\n");
  while (field)
  {
    headers.push(&headers, field, sizeof(char[strlen(field)]));
    field = strtok(NULL, "\n");
  }

  // Init the request's dictionary header_fields
  request->header_fields = dictionary_constructor(compare_string_keys);
  char *header = (char *)headers.peek(&headers);
  while (header)
  {
    char *key = strtok(header, ":");
    char *value = strtok(NULL, "\0");
    if (value)
    {
      if (value[0] == ' ')
      {
        value++; // Remove the leading white space
      }
      if (value[strlen(value) - 1] == '\r')
      {
        value[strlen(value) - 1] = '\0'; // Remove the trailing carriage return
      }
      request->header_fields.insert(&request->header_fields, key, sizeof(char[strlen(key)+1]), value, sizeof(char[strlen(value)+1]));
    }
    headers.pop(&headers, NULL);
    header = (char *)headers.peek(&headers);
  }

  queue_destructor(&headers, NULL);
}

/**
 * Parses the body according to the content type specified in the header fields.
 *
 * @param request The HTTPRequest struct that we're extracting the body from.
 * @param body The body of the request.
 */
void extract_body(struct HTTPRequest *request, char *body)
{
  char *content_type = (char *)request->header_fields.search(&request->header_fields, "Content-Type", sizeof("Content-Type"));
  struct Dictionary body_dict = dictionary_constructor(compare_string_keys);
  if (content_type)
  {
    if (strcmp(content_type, "application/x-www-form-urlencoded") == 0)
    {
      struct Queue fields = queue_constructor();
      char *field = strtok(body, "&");
      while (field)
      {
        field[strlen(field)] = '\0'; // Remove the trailing carriage return
        fields.push(&fields, field, sizeof(char[strlen(field)+1]));
        field = strtok(NULL, "&");
      }

      field = fields.peek(&fields);
      while (field)
      {
        char *key = strtok(field, "=");
        char *value = strtok(NULL, "\0");
        // Remove unnecessary leading white space.
        if (value[0] == ' ')
        {
          value++;
        }
        body_dict.insert(&body_dict, key, sizeof(char[strlen(key)]), value, sizeof(char[strlen(value)]));
        fields.pop(&fields, NULL);
        field = fields.peek(&fields);
      }
      queue_destructor(&fields, NULL);
    }
    else
    {
      body_dict.insert(&body_dict, "data", sizeof("data"), body, sizeof(char[strlen(body)]));
    }
  }
  request->body = body_dict;
}

void extract_request_query(struct HTTPRequest *request, char *query_string)
{
  struct Dictionary query_dict = dictionary_constructor(compare_string_keys);
  struct Queue fields = queue_constructor();
  char *field = strtok(query_string, "&");
  while (field)
  {
    fields.push(&fields, field, sizeof(char[strlen(field)]));
    field = strtok(NULL, "&");
  }

  field = fields.peek(&fields);
  while (field)
  {
    char *key = strtok(field, "=");
    char *value = strtok(NULL, "\0");
    // Remove unnecessary leading white space.
    if (value[0] == ' ')
    {
      value++;
    }
    query_dict.insert(&query_dict, key, sizeof(char[strlen(key)]), value, sizeof(char[strlen(value)]));
    fields.pop(&fields, NULL);
    field = fields.peek(&fields);
  }
  queue_destructor(&fields, NULL);

  request->query = query_dict;
}
