#ifndef TFTP_CLIENT_HANDLE_H
#define TFTP_CLIENT_HANDLE_H

#include "header.h"
#include "tftp_server.h"
#include "logger/logger.h"

#include <setjmp.h>

typedef struct _tftp_client_handler_
{
  struct TFTPServer server;
  struct sockaddr_in *_addr;
  int _block_size;
  int _window_size;
  int _check_addr;
  PacketBuffer *__last_packet;
  PacketBuffer *__packet_buffer;
  jmp_buf buf;
} TFTPClientHandler;

int handle_client(TFTPClientHandler *handler);
TFTPClientHandler *create_handler(const uint8_t *initial_buffer, size_t buffer_size, struct sockaddr_in *client_address, struct TFTPServer *server);
void free_handler(TFTPClientHandler *handler);

#endif
