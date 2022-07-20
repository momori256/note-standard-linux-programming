#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void ls(char *dir) {
  DIR *d = opendir(dir);
  if (!d) {
    perror("failed to open dir");
    _exit(1);
  }

  struct dirent *ent;
  while ((ent = readdir(d))) {
    printf("%s\n", ent->d_name);
  }
  closedir(d);
}

int sequal(char *s1, char *s2) { return strcmp(s1, s2) == 0; }

void ls_rec(char *dir, int dep) {
  DIR *d = opendir(dir);
  if (!d) {
    perror("failed to open dir");
    _exit(1);
  }

  char ident[256] = {0};
  for (int i = 0; i < dep; ++i) {
    ident[i] = '\t';
  }

  struct dirent *ent;
  while ((ent = readdir(d))) {
    char *name = ent->d_name;
    if (name[0] == '.') {
      continue;
    }
    char path[1024];
    sprintf(path, "%s/%s", dir, name);

    struct stat buf;
    if (lstat(path, &buf)) {
      perror("failed to get stat");
      _exit(1);
    }
    printf("%s%s\n", ident, name);
    if (S_ISDIR(buf.st_mode) && !(sequal(name, ".") || sequal(name, ".."))) {
      ls_rec(path, dep + 1);
    }
  }
  closedir(d);
  printf("\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: %s [-r] DIR...\n", argv[0]);
    return 0;
  }

  int is_rec = strcmp(argv[1], "-r") == 0;
  int si = is_rec ? 2 : 1;

  for (int i = si; i < argc; ++i) {
    if (is_rec) {
      ls_rec(argv[i], 0);
    } else {
      ls(argv[i]);
    }
  }

  return 0;
}