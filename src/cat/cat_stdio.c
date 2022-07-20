#include <stdio.h>
#include <unistd.h>

static int die(const char *const msg) {
  perror(msg);
  _exit(1);
  return 0;
}

int main(int argc, char *argv[]) {
  char buf[1024];
  FILE *fout = fdopen(STDOUT_FILENO, "w");

  for (int i = 1; i < argc; ++i) {
    FILE *f = fopen(argv[i], "r");
    (f != NULL) || die("failed to open");

    size_t r = fread(buf, sizeof(char), sizeof(buf), f);
    !ferror(f) || die("failed to read");

    size_t w = fwrite(buf, sizeof(char), r, fout);
    (w != 0) || die("failed to write");

    fclose(f);
  }

  fclose(fout);
  return 0;
}
