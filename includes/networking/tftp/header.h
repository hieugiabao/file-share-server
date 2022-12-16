#ifndef HEADER_H
#define HEADER_H

#include <netdb.h>

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
  INVALID_OPTIONS,
  INVALID_CHECKSUM,
} TFTPErrorCodes;

typedef struct _header_
{
  uint16_t checksum;
  uint16_t opcode;
} TFTPHeader;

typedef struct _tftp_packet_
{
  TFTPHeader header;
  uint8_t data[BUF_SIZE - sizeof(TFTPHeader)];
} Packet;

typedef struct _tftp_packet_buffer_
{
  Packet packet;
  ssize_t data_len;
  struct sockaddr_in *client_address;
  uint8_t _raw_data[BUF_SIZE];
  int block_id;
} PacketBuffer;

typedef struct _tftp_options_
{
  char name[256];
  char value[256];
  struct _tftp_options_ *next;
} TFTPOptions;

char *get_message(TFTPErrorCodes error_code);

#endif
