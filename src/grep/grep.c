#include <stdio.h>
#include <string.h>

int match_char(char c1, char c2) { return (c1 == c2) || (c2 == '.'); }

int is_repeat(const char *const s, int len, char c) {
  if (c == '.') {
    return 1;
  }

  const int n = strlen(s);
  if (n < len) {
    return 0;
  }

  for (int i = 0; i < len; ++i) {
    if (s[i] != c) {
      return 0;
    }
  }
  return 1;
}

int step_forward(const char **s, const char **p) {
  if (*(*p + 1) == '*') {
    char *sp = strchr(*s, *(*p + 2));
    int len = sp - *s;
    if (!is_repeat(*s, len, **p)) {
      return 0;
    }
    *s += len;
    *p += 2;
    return 1;
  }

  if (*(*p + 1) == '?') {
    char np = *(*p + 2);
    if (**s == **p) {
      *s += 1;
      *p += 2;
      return 1;
    }
    if (**s == np) {
      *s += 1;
      *p += 3;
      return 1;
    }
    return 0;
  }

  if (match_char(**s, **p)) {
    ++(*s), ++(*p);
    return 1;
  }
  return 0;
}

int match_part(const char *const s, const char *const p) {
  const char *sp = s;
  const char *pp = p;
  while ((*sp != '\0') && (*pp != '\0')) {
    if (!step_forward(&sp, &pp)) {
      return 0;
    }
  }
  return *pp == '\0';
}

int match_line(const char *const line, const char *const pat) {
  const int nl = strlen(line);

  for (int i = 0; i < nl; ++i) {
    if (match_part(line + i, pat)) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc > 3) {
    printf("usage: %s PATTERN [FILE]\n", argv[0]);
    return 0;
  }

  char *const pat = argv[1];
  { // remove meaningless patterns.
    int len = strlen(pat);
    char last = pat[len - 1];
    if (last == '*' || last == '?') {
      pat[len - 2] = '\0';
    }
  }

  FILE *f = (argc < 3) ? stdin : fopen(argv[2], "r");
  if (!f) {
    perror("failed to open");
    return 1;
  }

  char buf[1024];
  char line[1024];
  while (fgets(buf, sizeof(buf), f) != NULL) {
    strcpy(line, buf);
    line[strlen(line) - 1] = '\0';
    if (match_line(line, pat)) {
      if (fputs(buf, stdout) == EOF) {
        perror("failed to put output");
        return 1;
      }
    }
  }
  fclose(f);
  return 0;
}
