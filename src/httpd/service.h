#pragma once

#include "http.h"
#include <stdio.h>

struct Context {
  char *docrt;
  FILE *out;
};

int proc_req(struct Context ctx, struct Request req);

int service_main(int server_fd, char *docrt);
