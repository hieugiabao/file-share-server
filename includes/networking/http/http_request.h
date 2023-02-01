

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "data_structures/dictionary.h"

/**
 * The HTTPRequest struct contains three dictionaries representing the three components of an HTTP Request.
 */
struct HTTPRequest
{
  struct Dictionary request_line;  // The first line of an HTTP Request is the request line.
  struct Dictionary header_fields; // The header fields of an HTTP Request are the lines after the request line.
  struct Dictionary body;          // The body of an HTTP Request is the data after the header fields.
  struct Dictionary query;         // The query of an HTTP Request is the data after the question mark in the URL.
};

/* Constructor and destructor */

struct HTTPRequest http_request_constructor(char *request_string);

void http_request_destructor(struct HTTPRequest *http_request);

#endif // HTTP_REQUEST_H
