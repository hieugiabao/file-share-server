#ifndef TFTP_CLIENT_HANDLE_H
#define TFTP_CLIENT_HANDLE_H

#include "header.h"
#include "tftp_server.h"
#include "logger/logger.h"

#include <setjmp.h>

typedef struct _tftp_client_handler_
{
  struct TFTPServer server;      // The migrate server
  struct sockaddr_in *_addr;     // The address of the client
  int _block_size;               // The block size of the packet
  int _window_size;              // The window size of the packet
  int _check_addr;               // The flag to check the address
  PacketBuffer *__last_packet;   // The last packet
  PacketBuffer *__packet_buffer; // The packet buffer
  jmp_buf buf;                   // The exception handle signal
} TFTPClientHandler;

int handle_client(TFTPClientHandler *handler);
TFTPClientHandler *create_handler(const uint8_t *initial_buffer, size_t buffer_size, struct sockaddr_in *client_address, struct TFTPServer *server);
void free_handler(TFTPClientHandler *handler);

#endif
