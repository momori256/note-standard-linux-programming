#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

int cnt_nlines(FILE *f) {
  char c;
  int nlines = 0;
  while ((c = fgetc(f)) != EOF) {
    if (c == '\n')
      ++nlines;
  }
  rewind(f);
  return nlines;
}

void skip_lines(FILE *f, int nlines) {
  char c;
  while ((nlines > 0) && ((c = fgetc(f)) != EOF)) {
    if (c == '\n')
      --nlines;
  }
}

int write_all(FILE *f) {
  char c;
  while ((c = fgetc(f)) != EOF) {
    if (fputc(c, stdout) == EOF) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage: %s FILE NLINES\n", argv[0]);
    return 0;
  }

  FILE *fin = fopen(argv[1], "r");
  if (fin == NULL) {
    return 1;
  }

  int nlines = cnt_nlines(fin);
  int tail = atoi(argv[2]);
  skip_lines(fin, MAX(0, nlines - tail));
  write_all(fin);
  fclose(fin);

  return 0;
}
