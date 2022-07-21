#pragma once

#include <stdio.h>

#define LINE_SIZE (256)
#define FIELD_CNT (16)

#define CRLF "\r\n"
#define LF "\n"

struct Field {
  char key[LINE_SIZE];
  char value[LINE_SIZE];
};

struct Request {
  char method[8];
  char path[LINE_SIZE];
  struct Field fileds[FIELD_CNT];
};

void die(char *msg);

struct Request make_req(char buf[][LINE_SIZE]);

int read_req(FILE *in, char buf[][LINE_SIZE]);
