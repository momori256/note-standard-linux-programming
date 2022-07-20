#include <regex.h>
#include <stdio.h>

void grep(FILE *f, const regex_t *reg) {
  char buf[1024];
  while (fgets(buf, sizeof(buf), f)) {
    if (!regexec(reg, buf, 0, NULL, 0)) {
      fputs(buf, stdout);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("usage: %s PATTERN [FILE ...]\n", argv[0]);
    return 0;
  }

  regex_t reg;
  int err = regcomp(&reg, argv[1], REG_NOSUB | REG_NEWLINE);
  if (err) {
    char buf[1024];
    regerror(err, &reg, buf, sizeof(buf));
    puts(buf);
    return 1;
  }

  if (argc == 2) {
    grep(stdin, &reg);
    return 0;
  }

  for (int i = 2; i < argc; ++i) {
    FILE *f = fopen(argv[i], "r");
    if (!f) {
      perror("failed to open");
      continue;
    }
    grep(f, &reg);
  }

  regfree(&reg);
  return 0;
}
