#ifndef HEADER_H
#define HEADER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>

/* TFTP opcodes */
#define RRQ "\000\001"
#define WRQ "\000\002"
#define DATA "\000\003"
#define ACK "\000\004"
#define ERROR "\000\005"
#define OACK "\000\006"

/* Setting */
#define BLOCK_SIZE 512
#define MAX_RETRIES 10
#define TIMEOUT_SECOND 1
#define TIMEOUT_USECOND 500000
#define BUF_SIZE 65536

typedef enum _tfpt_error_codes_
{
  UNKNOWN,
  FILE_NOT_FOUND,
  ACCESS_VIOLATION,
  DISK_FULL,
  ILLEGAL_OPERATION,
  UNKNOWN_TRANSFER_ID,
  FILE_ALREADY_EXISTS,
  NO_SUCH_USER,
  INVALID_OPTIONS
} TFTPErrorCodes;

typedef struct _address_
{
  char host[256];
  char port[6];
} Address;

typedef struct _packet_buffer_
{
  uint8_t data[BUF_SIZE - 2];
  uint8_t opcode[2];
  ssize_t data_len;
  Address address;
} Packet;

typedef struct _tftp_options_
{
  char name[256];
  char value[256];
  struct _tftp_options_ *next;
} TFTPOptions;

char *get_message(TFTPErrorCodes error_code);

#endif