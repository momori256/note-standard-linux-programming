#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BACKLOG_CNT (5)

int listen_sock(char *port) {
  struct addrinfo *addr;

  struct addrinfo hints = {0};
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int err = 0;
  if ((err = getaddrinfo(NULL, port, &hints, &addr)) != 0) {
    return -1;
  }

  for (struct addrinfo *a = addr; a != NULL; a = a->ai_next) {
    int sock = socket(a->ai_family, a->ai_socktype, a->ai_protocol);
    if (bind(sock, a->ai_addr, a->ai_addrlen) < 0) {
      close(sock);
      continue;
    }
    if (listen(sock, BACKLOG_CNT) < 0) {
      close(sock);
      continue;
    }
    freeaddrinfo(addr);
    return sock;
  }

  return -1;
}
