#ifndef TFTP_CLIENT_HANDLE_H
#define TFTP_CLIENT_HANDLE_H

#include "header.h"
#include "logger/logger.h"

typedef struct _tftp_client_handler_ {
  int _sock;
  Address _addr;
  int _block_size;
  int _window_size;
  int _check_addr;
  Packet *__last_packet;
  Packet *__packet_buffer;
} TFTPClientHandler;

void handle_client(TFTPClientHandler *handler);
TFTPClientHandler *create_handler(const uint8_t *initial_buffer, size_t buffer_size, const char *client_address, const char *port);
void free_handler(TFTPClientHandler *handler);

#endif
