#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char buf[1024];
  if (argc == 1) {
    ssize_t r = read(STDIN_FILENO, buf, sizeof(buf));
    write(STDOUT_FILENO, buf, r);
    return 0;
  }

  for (int i = 1; i < argc; ++i) {
    int fd = open(argv[i], O_RDONLY);
    ssize_t r = read(fd, buf, sizeof(buf));
    if (r == -1) {
      perror("failed to read");
      _exit(1);
    }

    ssize_t w = write(STDOUT_FILENO, buf, r);
    if (w == -1) {
      perror("failed to write");
      _exit(1);
    }
    close(fd);
  }
  return 0;
}
