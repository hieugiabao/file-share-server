#ifndef HEADER_H
#define HEADER_H

#include <netdb.h>

/* TFTP opcodes */
#define RRQ "\000\001"   // Read request
#define WRQ "\000\002"   // Write request
#define DATA "\000\003"  // Data
#define ACK "\000\004"   // Acknowledgement
#define ERROR "\000\005" // Error
#define OACK "\000\006"  // Option acknowledgement

/* Setting */
#define BLOCK_SIZE 512         // The size of the data block
#define MAX_RETRIES 10         // The maximum number of retries
#define TIMEOUT_SECOND 1       // The timeout in seconds
#define TIMEOUT_USECOND 500000 // The timeout in microseconds
#define BUF_SIZE 65536         // The size of the buffer

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
  uint16_t checksum; // The checksum of the packet
  uint16_t opcode;   // The opcode of the packet
} TFTPHeader;        // The header of the packet

typedef struct _tftp_packet_
{
  TFTPHeader header;                           // The header of the packet
  uint8_t data[BUF_SIZE - sizeof(TFTPHeader)]; // The data of the packet
} Packet;                                      // The packet

typedef struct _tftp_packet_buffer_
{
  Packet packet;                      // The packet
  ssize_t data_len;                   // The length of the data
  struct sockaddr_in *client_address; // The address of the client
  uint8_t _raw_data[BUF_SIZE];        // The raw data of the packet
  int block_id;                       // The block id of the packet
} PacketBuffer;

typedef struct _tftp_options_
{
  char name[256];  // The name of the option
  char value[256]; // The value of the option
  struct _tftp_options_ *next;
} TFTPOptions;

char *get_message(TFTPErrorCodes error_code);

#endif
