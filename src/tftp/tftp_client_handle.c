#include "tftp/tftp_server.h"
#include <math.h>
#include <errno.h>
#include <ctype.h>

int __compare_address(const struct sockaddr_in *a1, const struct sockaddr_in *a2)
{
  if (a1->sin_addr.s_addr == a2->sin_addr.s_addr && a1->sin_port == a2->sin_port)
    return (1);
  return (0);
}

int _read_header(TFTPHeader *header, Packet *packet)
{
  TFTPHeader *tmp = (TFTPHeader *)packet;
  // header->checksum = tmp->checksum;
  header->opcode = tmp->opcode;
  return (0);
}

int _read_packet(uint8_t *dest, Packet *packet, uint16_t size)
{
  memcpy(dest, packet->data, size);
  return (0);
}

TFTPOptions *_process_option(TFTPClientHandler *handler, uint8_t *options);
void __map_options(TFTPClientHandler *handler, TFTPOptions *options);
void free_options(TFTPOptions *options);

void __recv_rq(TFTPClientHandler *handler, uint16_t *opcode, char *file_name, char *file_mode);
PacketBuffer *_recv_packet_mul(TFTPClientHandler *handler, uint16_t *opcodes, int min_data_length,
                               int handle_timeout);
PacketBuffer *_recv(TFTPClientHandler *handler, int handle_timeout);
PacketBuffer *_recv_packet(TFTPClientHandler *handler, uint16_t opcode, int min_data_length, int handle_timeout);
PacketBuffer *_recv_data(TFTPClientHandler *handler, int handle_timeout);
void _recv_ack(TFTPClientHandler *handler, int handle_timeout, int *block_id);
void _recv_file(TFTPClientHandler *handler, const char *filename);

void _send_err(TFTPClientHandler *handler, int error_code, char *error_message, struct sockaddr_in *addr);
void _send(TFTPClientHandler *handler, const uint8_t *data, ssize_t data_len, struct sockaddr_in *addr);
void _send_ack(TFTPClientHandler *handler, int block_number);
void _send_oack(TFTPClientHandler *handler, TFTPOptions *options);
void _send_file(TFTPClientHandler *handler, const char *filename, ssize_t buff_size);
void _send_data(TFTPClientHandler *handler, const uint8_t *data, ssize_t data_len, int block_id);
int __send_blocks(TFTPClientHandler *handler, const uint8_t *buffer, ssize_t buff_size, int outer_block_id, int inner_block_id);
void __resend_last_packet(TFTPClientHandler *handler);

char *_error_occurred(TFTPClientHandler *handler, int error_code, char *error_message, struct sockaddr_in *addr);
void _check_error(TFTPClientHandler *handler, PacketBuffer *packet, uint16_t *expected_opcodes);
void _terminate(TFTPClientHandler *handler, int error_code, const char *message, char *error_message);

void __handle_read(TFTPClientHandler *handler, const char *file_name);
void __handle_write(TFTPClientHandler *handler, const char *file_name);

TFTPClientHandler *create_handler(const uint8_t *initial_buffer, size_t buffer_size, struct sockaddr_in *client_address)
{
  TFTPClientHandler *handler = (TFTPClientHandler *)malloc(sizeof(TFTPClientHandler));
  handler->__packet_buffer = malloc(sizeof(Packet));
  handler->__last_packet = malloc(sizeof(Packet));
  handler->_block_size = BLOCK_SIZE;
  handler->_window_size = 1;
  handler->_check_addr = 1;
  handler->_addr = client_address;

  struct addrinfo hints,
      *servinfo, *p;
  int rv, yes = 1;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(server_host, NULL, &hints, &servinfo)) != 0)
  {
    log_error("getaddrinfo: %s", gai_strerror(rv));
    return NULL;
  }
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((handler->_sock = socket(p->ai_family, p->ai_socktype,
                                 p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }

    if (setsockopt(handler->_sock, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
      perror("setsockopt");
      return NULL;
    }

    if ((rv = bind(handler->_sock, p->ai_addr, p->ai_addrlen)) == -1)
    {
      close(handler->_sock);
      perror("server: bind");
      continue;
    }
    break;
  }

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(handler->_sock, (struct sockaddr *)&sin, &len) == -1)
    perror("getsockname");
  else
  {
    log_info("Incoming connection from (%s, %d). Binding at (%s, %d)", inet_ntoa(client_address->sin_addr), ntohs(client_address->sin_port), inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));

    // send new socket port to client
    uint16_t new_port = sin.sin_port;
    uint8_t *buffer = malloc(2 + 2);
    memcpy(buffer, ACK, 2);
    memcpy(buffer + 2, &new_port, 2);

    size_t byte_sent = sendto(sock_fd, buffer, 4, 0, (struct sockaddr *)client_address, sizeof(struct sockaddr_in));
    if (byte_sent < 1)
    {
      log_error("sendto() failed. (%s)", strerror(errno));
      return NULL;
    }

    free(buffer);
    close(sock_fd);

    // recv ack
    struct sockaddr_in peer_addr;
    socklen_t peer_addr_len = sizeof(peer_addr);
    buffer = malloc(4);
    ssize_t byte_received = recvfrom(handler->_sock, buffer, 4, 0, (struct sockaddr *)&peer_addr, &peer_addr_len);
    if (byte_received < 1)
    {
      log_error("recvfrom() failed. (%s)", strerror(errno));
      return NULL;
    }
    free(buffer);
  }

  freeaddrinfo(servinfo);
  struct timeval timeout = {.tv_sec = TIMEOUT_SECOND, .tv_usec = TIMEOUT_USECOND};
  if (setsockopt(handler->_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    perror("setsockopt set receive timeout failed\n");
  else if (setsockopt(handler->_sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    perror("setsockopt set send timeout failed\n");

  if (initial_buffer != NULL)
  {
    TFTPHeader *tmp = (TFTPHeader *)initial_buffer;
    // handler->__packet_buffer->packet.header.checksum = tmp->checksum;
    handler->__packet_buffer->packet.header.opcode = tmp->opcode;
    handler->__packet_buffer->client_address = client_address;
    memcpy(handler->__packet_buffer->packet.data, initial_buffer + sizeof(TFTPHeader), buffer_size - sizeof(TFTPHeader));
    handler->__packet_buffer->data_len = buffer_size - sizeof(TFTPHeader);
  }

  return handler;
}

void free_handler(TFTPClientHandler *handler)
{
  close(handler->_sock);
  free(handler->__packet_buffer);
  free(handler->__last_packet);
  free(handler);
}

char *_error_occurred(TFTPClientHandler *handler, int error_code, char *error_message, struct sockaddr_in *addr)
{
  if (error_message == NULL)
  {
    error_message = get_message(error_code);
  }
  _send_err(handler, error_code, error_message, addr);
  return error_message;
}

void _terminate(TFTPClientHandler *handler, int error_code, const char *message, char *error_message)
{
  error_message = _error_occurred(handler, error_code, error_message, NULL);
  close(handler->_sock);

  log_error("Terminated with error %d: %s; cause: %s", error_code, error_message, message);
  exit(1);
}

void _check_error(TFTPClientHandler *handler, PacketBuffer *packet_buffer, uint16_t *expected_opcodes)
{
  if (packet_buffer->packet.header.opcode == *(uint16_t *)ERROR)
  {
    uint16_t error_code;
    memcpy(&error_code, packet_buffer->packet.data, 2);
    uint8_t *error_message = malloc(packet_buffer->data_len - 2);
    memcpy(error_message, packet_buffer->packet.data + 2, packet_buffer->data_len - 2);
    log_error("%s - %s", get_message(error_code), error_message);
    exit(1);
  }

  // check opcode in expected opcodes
  uint16_t *tmp = expected_opcodes;
  while (tmp != NULL)
  {
    if (packet_buffer->packet.header.opcode == *tmp)
      return;
    tmp++;
  }
  char message[17 + packet_buffer->data_len];
  sprintf(message, "Invalid packet: ");
  memcpy(message + 17, packet_buffer->packet.data, packet_buffer->data_len);
  _terminate(handler, ILLEGAL_OPERATION, message, NULL);
}

PacketBuffer *_recv(TFTPClientHandler *handler, int handle_timeout)
{
  PacketBuffer *result;
  if (handler->__packet_buffer != NULL)
  {
    result = handler->__packet_buffer;
    handler->__packet_buffer = NULL;
    return result;
  }

  result = malloc(sizeof(PacketBuffer));
  struct sockaddr_in client_address;
  socklen_t client_len = sizeof(client_address);

  uint8_t data[BUF_SIZE];

  if (handle_timeout == 0)
  {
    ssize_t bytes_received = recvfrom(handler->_sock, data, BUF_SIZE, 0,
                                      (struct sockaddr *)&client_address, &client_len);
    result->client_address = &client_address;
    if (bytes_received == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        log_error("Timeout");
      }
      perror("recvfrom1");
      return NULL;
    }
    TFTPHeader *header = malloc(sizeof(TFTPHeader));
    _read_header(header, (Packet *)data);
    result->packet.header.opcode = ntohs(header->opcode);
    bzero(result->packet.data, BUF_SIZE - sizeof(TFTPHeader));
    _read_packet(result->packet.data, (Packet *)data, bytes_received - sizeof(TFTPHeader));
    result->data_len = bytes_received - sizeof(TFTPHeader);

    return result;
  }
  else
  {
    int retries = 0;

    while (retries <= MAX_RETRIES)
    {
      ssize_t bytes_received = recvfrom(handler->_sock, data, BUF_SIZE, 0,
                                        (struct sockaddr *)&client_address, &client_len);
      result->client_address = &client_address;
      if (bytes_received == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          log_error("Timeout, retrying... (%d/%d)", retries, MAX_RETRIES);
          retries++;
          if (retries <= MAX_RETRIES)
          {
            __resend_last_packet(handler);
          }
          continue;
        }
        else
        {
          perror("recvfrom");
          return NULL;
        }
      }
      else
      {
        TFTPHeader *header = malloc(sizeof(TFTPHeader));
        _read_header(header, (Packet *)data);
        result->packet.header.opcode = ntohs(header->opcode);
        bzero(result->packet.data, BUF_SIZE - sizeof(TFTPHeader));
        _read_packet(result->packet.data, (Packet *)data, bytes_received - sizeof(TFTPHeader));
        result->data_len = bytes_received - sizeof(TFTPHeader);

        return result;
      }
    }
  }

  return NULL;
}

PacketBuffer *_recv_packet_mul(TFTPClientHandler *handler, uint16_t *opcodes, int min_data_length,
                               int handle_timeout)
{
  PacketBuffer *packet_buffer;
  while (1)
  {
    packet_buffer = _recv(handler, handle_timeout);
    if (packet_buffer == NULL)
    {
      if (handle_timeout != 0)
      {
        log_error("Client %s: Timeout, no more retries.", inet_ntoa(handler->_addr->sin_addr));
        free_handler(handler);
        exit(1);
      }
      else
      {
        return NULL;
      }
    }
    if (handler->_check_addr == 0 || __compare_address(packet_buffer->client_address, handler->_addr) == 1)
      break;
    log_warn("Invalid TID: (%s, %hd) (expected: (%s, %hd))", inet_ntoa(packet_buffer->client_address->sin_addr),
             ntohs(packet_buffer->client_address->sin_port), inet_ntoa(handler->_addr->sin_addr),
             ntohs(handler->_addr->sin_port));
    _error_occurred(handler, UNKNOWN_TRANSFER_ID, NULL, packet_buffer->client_address);
  }

  if (!handler->_check_addr)
  {
    handler->_addr = packet_buffer->client_address;
    handler->_check_addr = 1;
  }

  _check_error(handler, packet_buffer, opcodes);
  if (packet_buffer->data_len < min_data_length)
  {
    char message[22 + min_data_length];
    sprintf(message, "Packet too short: %s", packet_buffer->packet.data);
    _terminate(handler, ILLEGAL_OPERATION, message, NULL);
  }
  // return value
  return packet_buffer;
}

void __recv_rq(TFTPClientHandler *handler, uint16_t *opcode, char *file_name, char *file_mode)
{
  uint16_t *opcodes;
  opcodes = malloc(2 * sizeof(uint16_t));
  opcodes[0] = *(uint16_t *)RRQ;
  opcodes[1] = *(uint16_t *)WRQ;

  PacketBuffer *packet_buffer = _recv_packet_mul(handler, opcodes, 2, 1);

  if (opcode == NULL)
  {
    opcode = malloc(2);
  }
  *opcode = packet_buffer->packet.header.opcode;

  // get file name
  uint8_t *data = packet_buffer->packet.data;
  if (file_name == NULL)
  {
    file_name = malloc(strlen((char *)data) + 1);
  }
  strcpy(file_name, (char *)data);

  data += strlen(file_name) + 1;
  if (file_mode == NULL)
  {
    file_mode = malloc(strlen((char *)data) + 1);
  }
  // log_info("Filename: %s", file_name);
  strcpy(file_mode, (char *)data);
  data += strlen(file_mode) + 1;

  // check file mode
  if (strcmp(file_mode, "octet") != 0)
  {
    _terminate(handler, ILLEGAL_OPERATION, "Only octet mode is supported.", NULL);
  }
  size_t current_len = ((size_t)2 + strlen(file_mode) + strlen(file_name));
  if ((int)current_len != (int)packet_buffer->data_len)
  {
    TFTPOptions *options = _process_option(handler, data);
    if (options != NULL)
    {
      __map_options(handler, options);
      _send_oack(handler, options);
      if (*opcode == *(uint16_t *)RRQ)
        _recv_ack(handler, 1, NULL);

      free_options(options);
    }
  }
}

void _recv_ack(TFTPClientHandler *handler, int handle_timeout, int *block_id)
{
  PacketBuffer *packet_buffer = _recv_packet(handler, *(uint16_t *)ACK, 2, handle_timeout);
  int id = (int)ntohs((*(uint16_t *)packet_buffer->packet.data));
  if (block_id != NULL)
  {
    *block_id = id;
  }
}

PacketBuffer *_recv_data(TFTPClientHandler *handler, int handle_timeout)
{
  PacketBuffer *packet_buffer = _recv_packet(handler, *(uint16_t *)DATA, 2, handle_timeout);
  uint16_t block_16;
  memcpy(&block_16, packet_buffer->packet.data, 2);
  packet_buffer->block_id = (int)ntohs(block_16);
  memcpy(packet_buffer->packet.data, packet_buffer->packet.data + 2, packet_buffer->data_len - 2);
  packet_buffer->data_len -= 2;
  return packet_buffer;
}

PacketBuffer *_recv_packet(TFTPClientHandler *handler, uint16_t opcode, int min_data_length, int handle_timeout)
{
  uint16_t *opcodes = malloc(1 * sizeof(uint16_t));
  opcodes[0] = opcode;

  return _recv_packet_mul(handler, opcodes, min_data_length, handle_timeout);
}

void _recv_file(TFTPClientHandler *handler, const char *filename)
{
  int last_id = 0;

  FILE *file = fopen(filename, "wb");

  if (file == NULL)
  {
    _terminate(handler, ACCESS_VIOLATION, "Cannot create file", NULL);
  }

  int retries = 0;
  while (retries <= MAX_RETRIES)
  {
    int start_last_id = last_id;
    for (int i = 0; i < handler->_window_size; i++)
    {
      PacketBuffer *packet_buffer = _recv_data(handler, 0);
      if (packet_buffer == NULL)
      {
        // timeout
        if (last_id == start_last_id)
        {
          retries++;
          break;
        }
        else
          retries = 0;
      }
      else
      {
        if (packet_buffer->block_id == last_id + 1)
        {
          last_id = packet_buffer->block_id;
          size_t bytes_written = fwrite(packet_buffer->packet.data, 1, packet_buffer->data_len, file);
          if (bytes_written != (size_t)packet_buffer->data_len)
          {
            // remove file
            fclose(file);
            remove(filename);
            if (errno == EFBIG || errno == ENOSPC)
              _terminate(handler, DISK_FULL, "Disk full", NULL);
            else
              _terminate(handler, UNKNOWN, "Unknown error", NULL);
          }

          if (packet_buffer->block_id == BUF_SIZE)
            last_id = -1;

          if (packet_buffer->data_len < handler->_block_size)
          {
            _send_ack(handler, last_id);
            fclose(file);
            return;
          }
        }
      }
    }

    if (retries <= MAX_RETRIES)
      _send_ack(handler, last_id == -1 ? BUF_SIZE : last_id);
  }
}

void _send(TFTPClientHandler *handler, const uint8_t *data, ssize_t data_len, struct sockaddr_in *addr)
{
  if (addr == NULL)
  {
    addr = handler->_addr;
  }
  PacketBuffer *last_packet = malloc(sizeof(PacketBuffer));
  last_packet->client_address = addr;
  memcpy(last_packet->packet.data, data, data_len);

  handler->__last_packet = last_packet;

  int retries = 0;

  while (retries <= MAX_RETRIES)
  {
    ssize_t byte_sent = sendto(handler->_sock, data, data_len, 0,
                               (struct sockaddr *)addr, sizeof(struct sockaddr_in));
    if (byte_sent == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        log_error("Timeout, retrying... (%d/%d)", retries, MAX_RETRIES);
        retries++;
        continue;
      }
      else
      {
        perror("sendto");
        free_handler(handler);
        exit(1);
      }
    }
    else
    {
      return;
    }
  }

  log_error("Client %s: Timeout, no more retries.", inet_ntoa(addr->sin_addr));
  free_handler(handler);
  exit(1);
}

void _send_oack(TFTPClientHandler *handler, TFTPOptions *options)
{
  uint8_t buffer[BUF_SIZE];
  memcpy(buffer, OACK, 2);
  int len = 2;
  TFTPOptions *current = options;
  while (current != NULL)
  {
    len += sprintf((char *)buffer + len, "%s%c%s%c", current->name, 0, current->value, 0);
    current = current->next;
  }

  _send(handler, buffer, len, NULL);
}

void _send_err(TFTPClientHandler *handler, int error_code, char *error_message, struct sockaddr_in *addr)
{
  char error_message_bytes[BUF_SIZE];
  uint16_t error_code_bytes = (uint16_t)error_code;
  memcpy(error_message_bytes, ERROR, 2);
  memcpy(error_message_bytes + 2, &error_code_bytes, 2);
  sprintf(error_message_bytes + 4, "%s%c", error_message, (uint8_t)0);
  _send(handler, (uint8_t *)error_message_bytes, strlen(error_message) + 5, addr);
}

void _send_file(TFTPClientHandler *handler, const char *filename, ssize_t buff_size)
{
  int outer_block_id = 0,
      block_id = 0;
  int ack_block_id, last_block_id;
  int byte_read = 0;

  FILE *file = fopen(filename, "rb");
  uint8_t buffer[handler->_block_size];

  while (1)
  {
    byte_read = fread(buffer, 1, handler->_block_size, file);
    if (byte_read < 0)
    {
      _terminate(handler, ACCESS_VIOLATION, "Cannot read file", NULL);
    }
    if (__send_blocks(handler, buffer, buff_size, outer_block_id, block_id) == 0)
    {
      fclose(file);
      return;
    }

    // receive ack
    _recv_ack(handler, 0, &ack_block_id);
    last_block_id = block_id + handler->_window_size;
    if ((last_block_id >= ack_block_id && ack_block_id >= block_id) || (ack_block_id <= (last_block_id % BUF_SIZE) && ack_block_id < block_id))
    {
      // If received ACK is a reply to one of the blocks sent
      // sent the next batch of blocks, else re-send
      if (ack_block_id < block_id)
        outer_block_id++;
      block_id = ack_block_id;
    }
  }
}

int __send_blocks(TFTPClientHandler *handler, const uint8_t *buffer, ssize_t buff_size, int outer_block_id, int inner_block_id)
{
  int i = 0, local_blkid,
      blk_size = handler->_block_size;

  for (i = 0; i < handler->_window_size; i++)
  {
    local_blkid = outer_block_id * BUF_SIZE + inner_block_id + i;
    if ((local_blkid * blk_size) > buff_size)
    {
      if (i == 0)
        return 0;
      else
        break;
    }
    ssize_t data_len_rest = buff_size - (local_blkid * blk_size);
    ssize_t data_len_send = data_len_rest < blk_size ? data_len_rest : blk_size;
    _send_data(handler, buffer, data_len_send, (local_blkid + 1) % BUF_SIZE);
  }

  return 1;
}

void _send_data(TFTPClientHandler *handler, const uint8_t *data, ssize_t data_len, int block_id)
{
  char *data_bytes = malloc(data_len + 4);
  memcpy(data_bytes, DATA, 2);
  uint16_t block_16 = ntohs((uint16_t)block_id);
  memcpy(data_bytes + 2, &block_16, 2);
  memcpy(data_bytes + 4, data, data_len);
  _send(handler, (uint8_t *)data_bytes, data_len + 4, NULL);
}

void _send_ack(TFTPClientHandler *handler, int block_number)
{
  char *ack_bytes = malloc(4);
  memcpy(ack_bytes, ACK, 2);
  uint16_t block_16 = ntohs((uint16_t)block_number);
  memcpy(ack_bytes + 2, &block_16, 2);
  _send(handler, (uint8_t *)ack_bytes, 4, NULL);
}

void __resend_last_packet(TFTPClientHandler *handler)
{
  _send(handler, handler->__last_packet->packet.data, handler->__last_packet->data_len, NULL);
}

void handle_client(TFTPClientHandler *handler)
{
  uint16_t opcode;
  char file_name[100];
  char file_mode[10];

  __recv_rq(handler, &opcode, file_name, file_mode);

  if (opcode == *(uint16_t *)RRQ)
  {
    log_info("Client (%s:%d): Request get file %s.", inet_ntoa(handler->_addr->sin_addr), ntohs(handler->_addr->sin_port), file_name);
    __handle_read(handler, file_name);
  }
  else if (opcode == *(uint16_t *)WRQ)
  {
    log_info("Client (%s:%d): Request put file %s.", inet_ntoa(handler->_addr->sin_addr), ntohs(handler->_addr->sin_port), file_name);
    __handle_write(handler, file_name);
  }
  else
  {
    _terminate(handler, ILLEGAL_OPERATION, "Invalid opcode", NULL);
  }
}

void __handle_read(TFTPClientHandler *handler, const char *file_name)
{

  FILE *fileptr;
  long filelen;

  char full_path[128];
  sprintf(full_path, "upload/%s", file_name);
  if (access(full_path, F_OK) != 0)
  {
    _terminate(handler, FILE_NOT_FOUND, "File not found", NULL);
  }

  fileptr = fopen(full_path, "rb"); // Open the file in binary mode
  if (fileptr == NULL)
  {
    _terminate(handler, FILE_NOT_FOUND, "File not found", NULL);
  }

  fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
  filelen = ftell(fileptr);    // Get the current byte offset in the file
  rewind(fileptr);             // Jump back to the beginning of the file

  fclose(fileptr); // Close the file

  _send_file(handler, full_path, filelen);
}

void __handle_write(TFTPClientHandler *handler, const char *file_name)
{
  if (is_upload_allowed == 0)
  {
    _terminate(handler, ACCESS_VIOLATION, "Upload not allowed", NULL);
  }
  char full_path[128];
  sprintf(full_path, "%s/%s", upload_dir, file_name);

  if (access(full_path, F_OK) == 0)
  {
    _terminate(handler, FILE_ALREADY_EXISTS, "File already exists", NULL);
  }

  _send_ack(handler, 0);
  _recv_file(handler, full_path);
}

TFTPOptions *_process_option(TFTPClientHandler *handler, uint8_t *options)
{
  if (options == NULL)
  {
    return NULL;
  }
  TFTPOptions *head = NULL, *current;
  uint8_t *tmp = options;
  int i = 0;
  while (isprint((int)*tmp) != 0)
  {
    TFTPOptions *option = malloc(sizeof(TFTPOptions));
    option->next = NULL;
    strcpy(option->name, (char *)tmp);
    tmp = tmp + strlen((char *)tmp) + 1;

    if (tmp == NULL)
    {
      _terminate(handler, ILLEGAL_OPERATION, "Invalid options received", NULL);
    }
    else
    {
      strcpy(option->value, (char *)tmp);
      tmp = tmp + strlen((char *)tmp) + 1;
    }

    if (head == NULL)
    {
      head = option;
      current = option;
    }
    else
    {
      current->next = option;
      current = option;
    }
    i++;
    if (i == 2)
      exit(0);
  }

  return head;
}

void __map_options(TFTPClientHandler *handler, TFTPOptions *options)
{
  TFTPOptions *current = options;

  while (current != NULL)
  {
    if (strcmp(current->name, "blksize") == 0)
    {
      handler->_block_size = atoi(current->value);
      if (handler->_block_size < 0 || handler->_block_size > 65464)
      {
        _terminate(handler, ILLEGAL_OPERATION, "Invalid block size", NULL);
      }
    }

    if (strcmp(current->name, "windowsize") == 0)
    {
      handler->_window_size = atoi(current->value);
      if (handler->_window_size < 0 || handler->_window_size > 255)
      {
        _terminate(handler, ILLEGAL_OPERATION, "Invalid window size", NULL);
      }
    }

    current = current->next;
  }
}

void free_options(TFTPOptions *options)
{
  TFTPOptions *current = options;
  while (current != NULL)
  {
    TFTPOptions *tmp = current;
    current = current->next;
    free(tmp);
  }
}
