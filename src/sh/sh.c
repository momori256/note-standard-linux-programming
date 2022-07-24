#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

void prompt();

int main(int argc, char *argv[]) {
  while (1) {
    prompt();
  }
  return 0;
}

#define MAX_ARGC (32)
#define MAX_LINE (1024)
#define MAX_CMD_NAME (12)

#define PIPE "|"
#define RE_I "<"
#define RE_O ">"

struct Cmd {
  int argc;
  char *argv[MAX_ARGC];
  char *rei;
  char *reo;
  pid_t pid;
  struct Cmd *next;
};

int is_ident(const char *const s) {
  return (strncmp(s, RE_I, sizeof(RE_I) - 1) == 0) ||
         (strncmp(s, RE_O, sizeof(RE_O) - 1) == 0) ||
         (strncmp(s, PIPE, sizeof(PIPE) - 1) == 0);
}

char *skip_whitesp(char *p) { return p + strspn(p, " \t"); }

#define VALIDATE_CMD(cmd)                                                      \
  if (!cmd) {                                                                  \
    fprintf(stderr, "[%s] cmd is NULL (line: %s)\n", __func__, line);          \
    exit(1);                                                                   \
  }

void die(char *s) {
  perror(s);
  exit(1);
}

char *seek_arg_end(const char *const p) {
  char *t = strchr(p, ' ');
  if (t) {
    return t;
  }
  return strchr(p, '\0');
}

void parse_cmdname(struct Cmd *cmd, const char *const line) {
  VALIDATE_CMD(cmd);

  char buf[MAX_CMD_NAME];
  strncpy(buf, line, sizeof(buf));

  char *p = seek_arg_end(buf);
  *p = '\0';

  cmd->argv[0] = (char *)malloc(sizeof(char) * (strlen(buf) + 1));
  strcpy(cmd->argv[0], buf);

  cmd->argc = 1;
}

void parse_redirect(struct Cmd *cmd, const char *const line, char *re) {
  VALIDATE_CMD(cmd);

  char buf[MAX_LINE];
  strncpy(buf, line, sizeof(buf));

  char *p = strstr(buf, re);
  if (!p) {
    return;
  }

  {
    char *pipe = strstr(buf, PIPE);
    if ((pipe != NULL) && (pipe < p)) {
      return;
    }
  }

  char *beg = skip_whitesp(p + 1);
  p = seek_arg_end(beg);
  *p = '\0';

  if (strcmp(re, RE_I) == 0) {
    cmd->rei = (char *)malloc(sizeof(char) * (strlen(beg) + 1));
    strcpy(cmd->rei, beg);
  } else if (strcmp(re, RE_O) == 0) {
    cmd->reo = (char *)malloc(sizeof(char) * (strlen(beg) + 1));
    strcpy(cmd->reo, beg);
  }
}

void parse_args(struct Cmd *cmd, const char *const line) {
  VALIDATE_CMD(cmd);

  char buf[MAX_LINE];
  strncpy(buf, line, sizeof(buf));

  char *p = seek_arg_end(buf); // strchr(buf, ' ');
  if (!p) {
    return;
  }
  p = skip_whitesp(p + 1);

  while ((p != NULL) && (*p != '\0') && !is_ident(p)) {
    char *t = p;
    p = seek_arg_end(t);
    *p++ = '\0';
    cmd->argv[cmd->argc] = (char *)malloc(sizeof(char) * (strlen(t) + 1));
    strcpy(cmd->argv[cmd->argc], t);
    ++cmd->argc;
  }
}

struct Cmd *parse_cmds(char line[]);

void parse_pipe(struct Cmd *cmd, const char *const line) {
  VALIDATE_CMD(cmd);

  char *pipe = strstr(line, PIPE);
  if (!pipe) {
    return;
  }

  char *nx_line = skip_whitesp(pipe + 1);
  cmd->next = parse_cmds(nx_line);
}

struct Cmd *parse_cmds(char line[]) {
  struct Cmd *cmd = (struct Cmd *)malloc(sizeof(struct Cmd));

  parse_cmdname(cmd, line);
  parse_args(cmd, line);
  parse_redirect(cmd, line, RE_I);
  parse_redirect(cmd, line, RE_O);
  parse_pipe(cmd, line);

  return cmd;
}

void print_prompt() {
  char dir[256];
  getcwd(dir, sizeof(dir));
  printf("[%s] $> ", dir);
  fflush(stdout);
}

void move_fd(int src, int dst) {
  close(dst);
  dup2(src, dst);
  close(src);
}

void builtin_cd(const char *const dir) {
  if (chdir(dir) == -1) {
    perror("chdir");
  }
}

void builtin_exit() { exit(0); }

void exec_cmd(struct Cmd *cmd) {
  int fds_cur[2] = {-1, -1};
  int fds_prev[2] = {-1, -1};

  for (struct Cmd *c = cmd; c != NULL; c = c->next) {
    fds_prev[0] = fds_cur[0];
    fds_prev[1] = fds_cur[1];

    { // builtin commands (not used with pipe).
      if (strcmp(c->argv[0], "exit") == 0) {
        builtin_exit();
        continue;
      }
      if (strcmp(c->argv[0], "cd") == 0) {
        builtin_cd(c->argv[1]);
        continue;
      }
    }

    if (c->next) {
      if (pipe(fds_cur) == -1) {
        die("pipe");
      }
    }

    pid_t pid = fork();
    if (pid == -1) {
      die("fork");
    }

    if (pid > 0) { // parent.
      if (fds_prev[0] != -1) {
        close(fds_prev[0]);
      }
      if (fds_prev[1] != -1) {
        close(fds_prev[1]);
      }
      c->pid = pid;
      continue;
    }

    if (c->rei) {
      int fd = open(c->rei, O_RDONLY);
      if (fd == -1) {
        die("open");
      }
      move_fd(fd, STDIN_FILENO);
    }

    if (c->reo) {
      int fd = open(c->reo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd == -1) {
        die("open");
      }
      move_fd(fd, STDOUT_FILENO);
    }

    // pipe.
    if (c != cmd) {
      close(fds_prev[1]);
      move_fd(fds_prev[0], 0); // fd from prev command.
    }

    if (c->next) {
      close(fds_cur[0]);
      move_fd(fds_cur[1], 1); // fd to next command.
    }

    execvp(c->argv[0], c->argv);
    die("execvp");
  }
}

void wait_cmd(struct Cmd *cmd) {
  for (struct Cmd *c = cmd; c != NULL; c = c->next) {
    waitpid(c->pid, NULL, 0);
  }
}

void free_cmd(struct Cmd *cmd) {
  for (int i = 0; i < cmd->argc; ++i) {
    free(cmd->argv[i]);
    cmd->argv[i] = NULL;
  }
  if (cmd->rei) {
    free(cmd->rei);
    cmd->rei = NULL;
  }
  if (cmd->reo) {
    free(cmd->reo);
    cmd->reo = NULL;
  }
  if (cmd->next) {
    free_cmd(cmd->next);
    cmd->next = NULL;
  }
  free(cmd);
  cmd = NULL;
}

void prompt() {
  print_prompt();

  char buf[MAX_LINE];
  if (!fgets(buf, sizeof(buf), stdin)) {
    die("fgets");
  }
  buf[strlen(buf) - 1] = '\0';

  struct Cmd *cmd = parse_cmds(buf);

  exec_cmd(cmd);
  wait_cmd(cmd);
  free_cmd(cmd);
}
