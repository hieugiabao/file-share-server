
#include "http/helper/helper.h"
#include "model/session.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

uint32_t base64_chars_strchr(char chr);
void build_decoding_table();

char *format_404()
{
  return "HTTP/1.1 404 Not Found\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_403()
{
  return "HTTP/1.1 403 Forbidden\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_401()
{
  return "HTTP/1.1 401 Unauthorized\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_400()
{
  return "HTTP/1.1 400 Bad Request\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_500()
{
  return "HTTP/1.1 500 Internal Server Error\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_501()
{
  return "HTTP/1.1 501 Not Implemented\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_505()
{
  return "HTTP/1.1 505 HTTP Version Not Supported\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_422()
{
  return "HTTP/1.1 422 Unprocessable Entity\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_409()
{
  return "HTTP/1.1 409 Conflict\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_200()
{
  return "HTTP/1.1 200 OK\r\n"
         "Connection: close\r\n"
         "Content-Length: 0\r\n\r\n";
}

char *format_200_with_content(char *content)
{
  char *response = malloc(strlen(content) + 100);
  sprintf(response, "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Length: %ld\r\n"
                    "Content-Type: text/html\r\n\r\n%s\r\n",
          strlen(content), content);
  return response;
}

char *format_200_with_content_type(char *content, char *content_type)
{
  char *response = malloc(strlen(content) + 100);
  sprintf(response, "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Length: %ld\r\n"
                    "Content-Type: %s\r\n\r\n%s\r\n",
          strlen(content), content_type, content);
  return response;
}

char *format_200_with_content_type_and_length(char *content, char *content_type, int content_length)
{
  char *response = malloc(strlen(content) + 100);
  sprintf(response, "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Length: %d\r\n"
                    "Content-Type: %s\r\n\r\n%s\r\n",
          content_length, content_type, content);
  return response;
}

struct User *get_user_from_request(struct HTTPRequest *request, char *token)
{
  char *auth_header = request->header_fields.search(&request->header_fields, "Authorization", sizeof(char[strlen("Authorization")+1]));

  if (auth_header == NULL)
  {
    return NULL;
  }

  char *token_type = strtok(auth_header, " ");
  if (strcmp(token_type, "Basic") != 0)
  {
    return NULL;
  }

  char *token_value = strtok(NULL, "\0");
  if (token_value == NULL)
  {
    return NULL;
  }

  size_t length = 0;
  unsigned char *_decoded_token = base64_decode(token_value, strlen(token_value), &length);
  
  if (_decoded_token == NULL)
  {
    return NULL;
  }
  char decoded_token[length + 1];
  memcpy(decoded_token, _decoded_token, length);
  decoded_token[length] = '\0';
  if (token != NULL)
  {
    strcpy(token, (char *)decoded_token);
  }
  struct Session *session = session_find_by_token((char *)decoded_token);
  if (session == NULL)
  {
    return NULL;
  }

  // check session expiration
  if (session->is_expired(session) == 1)
  {
    session->delete_session(session);
    session_free(session);
    return NULL;
  }
  struct User *user = malloc(sizeof(struct User));
  memcpy(user, session->get_user(session), sizeof(struct User));
  session_free(session);

  return user;
}

/**
 * It generates a random string of characters
 *
 * @param str The string to be generated.
 * @param size The size of the string to be generated.
 *
 * @return A string of random characters.
 */
char *generate_token(char *str, size_t size)
{
  const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  if (size)
  {
    --size;
    for (size_t n = 0; n < size; n++)
    {
      size_t key = (double)rand() / RAND_MAX * (sizeof charset - 1);
      str[n] = charset[key];
    }
    str[size] = '\0';
  }
  return str;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length)
{

  *output_length = 4 * ((input_length + 2) / 3);

  char *encoded_data = malloc(*output_length);
  if (encoded_data == NULL)
    return NULL;

  for (size_t i = 0, j = 0; i < input_length;)
  {

    uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[*output_length - 1 - i] = '=';

  return encoded_data;
}

unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length)
{
  if (decoding_table == NULL)
    build_decoding_table();

  if (input_length % 4 != 0)
    return NULL;

  *output_length = input_length / 4 * 3;
  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = malloc(*output_length);
  if (decoded_data == NULL)
    return NULL;

  for (size_t i = 0, j = 0; i < input_length;)
  {

    uint32_t sextet_a = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(int)data[i++]];
    uint32_t sextet_b = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(int)data[i++]];
    uint32_t sextet_c = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(int)data[i++]];
    uint32_t sextet_d = data[i] == '=' ? 0 & i++ : (uint32_t)decoding_table[(int)data[i++]];

    uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

    if (j < *output_length)
      decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *output_length)
      decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

/**
 * It returns the index of a character in the base64 character set
 *
 * @param chr The character to be searched for.
 *
 * @return The index of the character in the base64 character set.
 */
uint32_t base64_chars_strchr(char chr)
{
  const unsigned char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  for (uint32_t i = 0; i < sizeof base64_chars - 1; i++)
  {
    if (base64_chars[i] == chr)
    {
      return i;
    }
  }
  return -1;
}

void build_decoding_table()
{

  decoding_table = malloc(256);

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char)encoding_table[i]] = i;
}

void base64_cleanup()
{
  free(decoding_table);
}
