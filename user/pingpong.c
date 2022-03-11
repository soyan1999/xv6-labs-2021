#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char buf = 'a';
  int pipe1[2];
  int pipe2[2];

  pipe(pipe1);
  pipe(pipe2);

  if (fork() == 0) {
    close(pipe1[1]);
    close(pipe2[0]);

    read(pipe1[0], &buf, 1);
    printf("%d: received ping\n", getpid());
    write(pipe2[1], &buf, 1);

    close(pipe1[0]);
    close(pipe2[1]);

    exit(0);
  }

  close(pipe1[0]);
  close(pipe2[1]);

  write(pipe1[1], &buf, 1);
  read(pipe2[0], &buf, 1);
  printf("%d: received pong\n", getpid());

  close(pipe1[1]);
  close(pipe2[0]);

  exit(0);
}
