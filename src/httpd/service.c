#include "service.h"
#include "http.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

void no_action(int sig) {}

// Do not transform childlen into zombies.
int detach_children() {
  struct sigaction act;
  act.sa_handler = no_action;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
  if (sigaction(SIGCHLD, &act, NULL) < 0) {
    return 1;
  }
  return 0;
}

int get_file_size(const char *const path) {
  struct stat s;
  if (lstat(path, &s)) {
    return 0;
  }
  return (int)s.st_size;
}

int proc_req(struct Context ctx, struct Request req) {
  if (strcmp(req.method, "GET") == 0) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s%s", ctx.docrt, req.path);
    FILE *f = fopen(path, "r");
    if (!f) {
      fprintf(ctx.out, "HTTP/1.0 404 Not Found%s%s", CRLF, CRLF);
      return 1;
    }

    fprintf(ctx.out, "HTTP/1.0 200 OK%s", CRLF);
    fprintf(ctx.out, "Content-Length: %d%s", get_file_size(path), CRLF);
    fprintf(ctx.out, "%s", CRLF);

    char buf[1024];
    int read = 0;
    while ((read = fread(buf, sizeof(char), sizeof(buf), f))) {
      if (!fwrite(buf, sizeof(char), read, ctx.out))
        die("fwrite failed");
    }
  }
  return 0;
}

int service_main(int server_fd, char *docrt) {
  // syslog can be read with journalctl.
  openlog("my_httpd", 0, LOG_USER);

  detach_children();

  while (1) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof(addr);

    int sock = accept(server_fd, (struct sockaddr *)&addr, &addrlen);
    if (sock < 0) {
      return 1;
    }

    syslog(LOG_INFO, "accept connection");

    pid_t pid = fork();
    if (pid) { // parent.
      close(sock);
      continue;
    }

    FILE *in = fdopen(sock, "r");
    if (!in) {
      perror("in");
      return 1;
    }
    FILE *out = fdopen(sock, "w");
    if (!out) {
      perror("out");
      return 1;
    }

    char buf[32][LINE_SIZE];
    read_req(in, buf);
    struct Request req = make_req(buf);
    struct Context ctx = {
        .docrt = docrt,
        .out = out,
    };
    proc_req(ctx, req);

    fflush(out);
    fclose(in);
    fclose(out);

    syslog(LOG_INFO, "close connection");
  }
  return 0;
}
