#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

struct option longopts[] = {
    {"lines", required_argument, NULL, 'n'},
    {"help", no_argument, NULL, 'h'},
    {0},
};

static int die(const char *const msg) {
  perror(msg);
  _exit(1);
  return 0;
}

static int head(FILE *fout, FILE *fin, int nlines) {
  if (fout == NULL || fin == NULL) {
    return -1;
  }

  char c;
  while ((nlines > 0) && ((c = fgetc(fin)) != EOF)) {
    if (c == '\n')
      --nlines;
    if (fputc(c, fout) == EOF) {
      return -1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  int opt;
  int nlines = 10;
  while ((opt = getopt_long(argc, argv, "n:h", longopts, NULL)) != -1) {
    switch (opt) {
    case 'n':
      nlines = atol(optarg);
      break;
    case 'h':
    case '?':
      printf("usage: %s [-n LINES] [FILE]\n", argv[0]);
      return 0;
    }
  }

  if (optind == argc) {
    head(stdout, stdin, nlines);
    return 0;
  }

  for (int i = optind; i < argc; ++i) {
    FILE *f = fopen(argv[i], "r");
    if (!f) {
      die("failed to open input file");
    }
    head(stdout, f, nlines);
    fclose(f);
  }

  return 0;
}
