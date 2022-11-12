
#include "tftp/tftp_server.h"

int sock_fd = -1; // listen on sock_fd
int is_upload_allowed = 0;
char *upload_dir;
char *server_host;

void _sigchld_handler()
{
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
}

int init_server(const char *host, const char *port, const char *root_dir, int allow_upload)
{
  struct addrinfo hints, *servinfo, *p;
  struct sigaction sa;
  int yes = 1, rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
  {
    log_fatal("getaddrinfo: %s", gai_strerror(rv));
    return -1;
  }

  // loop through all the results and bind to the first we can
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sock_fd = socket(p->ai_family, p->ai_socktype,
                          p->ai_protocol)) == -1)
    {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                   sizeof(int)) == -1)
    {
      perror("setsockopt");
      return -1;
    }

    if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(sock_fd);
      perror("server: bind");
      continue;
    }

    break;
  }

  if (p == NULL)
  {
    log_fatal("server: failed to bind");
    return -2;
  }

  freeaddrinfo(servinfo); // all done with this structure

  sa.sa_handler = _sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1)
  {
    perror("sigaction");
    return -1;
  }

  log_info("Starting TFTP server, listening on %s at port %s", host, port);
  upload_dir = strdup(root_dir);
  server_host = strdup(host);
  is_upload_allowed = allow_upload == 0 ? 0 : 1;
  return 0;
}

void serve()
{
  if (sock_fd == -1)
  {
    log_fatal("Server is not initialized");
    return;
  }

  log_info("Waiting for connections...");
  while (1)
  {
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    uint8_t data[BUF_SIZE];
    int bytes_received = recvfrom(sock_fd, data, BUF_SIZE, 0,
                                  (struct sockaddr *)&client_address, &client_len);
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, client_len,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST | NI_NUMERICSERV);
    if (bytes_received < 1)
    {
      log_error("connection closed by client");
      continue;
    }
    if (!fork()) // this is the child process
    {
      close(sock_fd); // child doesn't need the listener
      TFTPClientHandler *handler = create_handler(data, bytes_received, address_buffer, service_buffer);
      handle_client(handler);
      log_info("Closing connection with %s:%s! Done request", address_buffer, service_buffer);
      free_handler(handler);
      exit(0);
    }
  }
}

int close_server()
{
  if (sock_fd == -1)
  {
    log_fatal("Server is not initialized");
    return -1;
  }
  close(sock_fd);
  sock_fd = -1;
  return 0;
}