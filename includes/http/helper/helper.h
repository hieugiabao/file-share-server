
#ifndef _HTTP_HELPER_H_
#define _HTTP_HELPER_H_

#include "model/user.h"
#include "networking/http/http_request.h"

char *format_404();

char *format_403();

char *format_409();

char *format_401();

char *format_400();

char *format_500();

char *format_501();

char *format_505();

char *format_422();

char *format_200();

char *format_200_with_content(char *content);

char *format_200_with_content_type(char *content, char *content_type);

char *format_200_with_content_type_and_length(char *content, char *content_type, int content_length);

struct User *get_user_from_request(struct HTTPRequest *request, char *token);

char *generate_token(char *str, size_t size);

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length);

#endif
