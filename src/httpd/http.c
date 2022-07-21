#include <fcntl.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

#include "http.h"

const char *const GET = "GET";
const char *const HEAD = "HEAD";

void die(char *msg) {
  perror(msg);
  exit(1);
}

int is_empty_line(const char *const s) {
  return (strcmp(s, CRLF) == 0) || (strcmp(s, LF) == 0);
}

void reqtos(char *const buf, struct Request req) {
  sprintf(buf, "method: %s\npath: %s", req.method, req.path);
}

struct Field make_field(const char *const s) {
  struct Field fld = {0};

  char t[LINE_SIZE];
  strncpy(t, s, sizeof(t));

  char *p = strchr(t, ':');
  *p++ = '\0';
  strncpy(fld.key, t, sizeof(fld.key));

  p += strspn(p, " \t");
  strncpy(fld.value, p, sizeof(fld.value));

  return fld;
}

struct Request make_req(char buf[][LINE_SIZE]) {
  struct Request req = {0};

  {
    char line[LINE_SIZE];
    strncpy(line, buf[0], LINE_SIZE);
    char *p = strchr(line, ' ');
    if (p == NULL)
      die("invalid request header");
    *p++ = '\0';
    strcpy(req.method, line);

    char *p2 = strchr(p, ' ');
    if (p2 == NULL)
      die("invalid request header");
    *p2 = '\0';
    strncpy(req.path, p, LINE_SIZE);
  }

  {
    int cnt = 0;
    char *s = buf[cnt + 1];
    while (!is_empty_line(s)) {
      req.fileds[cnt++] = make_field(s);
      s = buf[cnt + 1];
    }
  }
  return req;
}

int read_req(FILE *in, char buf[][LINE_SIZE]) {
  int cnt = 0;
  while (1) {
    if (!fgets(buf[cnt], sizeof(buf[cnt]), in)) {
      die("fgets failed");
    }
    if (is_empty_line(buf[cnt])) {
      break;
    }
    ++cnt;
  }
  return 0;
}
