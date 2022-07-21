#include "http.h"
#include "service.h"
#include "socket.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("usage: %s DOCUMENT_ROOT\n", argv[0]);
    return 0;
  }

  int server_fd = listen_sock("8080");
  service_main(server_fd, argv[1]);
  close(server_fd);
  return 0;
}
