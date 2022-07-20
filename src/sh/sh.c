#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int process(int argc, char *argv[]);
void die(char *s);
int find(int len, char *v[], const char *const key);
int find_last_arg_idx(int argc, char *argv[]);
void mv_pipe(int srcfd, int dstfd);

int main(int argc, char *argv[]) {
  process(argc - 1, argv + 1);
  return 0;
}

const char *const REI = "<-";  // redirect in.
const char *const REO = "->";  // redirect out.
const char *const PIPE = "|>"; // pipe.

int process(int argc, char *argv[]) {
  fprintf(stderr, "[process] argc: %d, argv[0]: %s\n", argc, argv[0]);

  pid_t pid = fork();

  if (pid) { // parent.
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      fprintf(stderr, "pid: %d, status: %d.\n", pid, WEXITSTATUS(status));
      return 0;
    }
    if (WIFSIGNALED(status)) {
      fprintf(stderr, "pid: %d, signal: %d.\n", pid, WTERMSIG(status));
      return 0;
    }
    fprintf(stderr, "pid: %d, terminated abnormaly.\n", pid);
    return 0;
  }

  int arg_idx = find_last_arg_idx(argc, argv);
  char *args[arg_idx + 1];
  {
    for (int i = 0; i < arg_idx + 1; ++i) {
      args[i] = argv[i];
    }
    args[arg_idx] = NULL;
  }

  int ri_idx = find(argc, argv, REI);
  if (ri_idx != -1) {
    int fd = open(argv[ri_idx + 1], O_RDONLY);
    if (fd == -1) {
      die("open input redirect failed");
    }
    mv_pipe(fd, STDIN_FILENO);
  }

  int ro_idx = find(argc, argv, REO);
  if (ro_idx != -1) {
    int fd = open(argv[ro_idx + 1], O_WRONLY);
    if (fd == -1) {
      die("open output redirect failed");
    }
    mv_pipe(fd, STDOUT_FILENO);
  }

  int p_idx = find(argc, argv, PIPE);
  if (p_idx == -1) {
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
  }

  // pipe.
  int fds[2];
  if (p_idx != -1) {
    if (pipe(fds)) {
      die("pipe failed");
    }
  }

  pid_t pid2 = fork();
  if (pid2) { // parent.
    close(fds[0]);
    mv_pipe(fds[1], STDOUT_FILENO);
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
  }

  close(fds[1]);
  mv_pipe(fds[0], STDIN_FILENO);
  process(argc - (p_idx + 1), argv + p_idx + 1);
  return 0;
}

void die(char *s) {
  perror(s);
  exit(1);
}

int find(int len, char *v[], const char *const key) {
  for (int i = 0; i < len; ++i) {
    if (strcmp(v[i], key) == 0) {
      return i;
    }
  }
  return -1;
}

int find_last_arg_idx(int argc, char *argv[]) {
  int arg_idx = 1;
  while ((arg_idx < argc) &&
         (strcmp(argv[arg_idx], REI) != 0 && strcmp(argv[arg_idx], REO) != 0 &&
          strcmp(argv[arg_idx], PIPE) != 0)) {
    ++arg_idx;
  }
  return arg_idx;
}

void mv_pipe(int srcfd, int dstfd) {
  close(dstfd);
  dup2(srcfd, dstfd);
  close(srcfd);
}
