#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void primes(int *p) {
  int lead, buf, p0[2];
  close(p[1]);
  if (!read(p[0], &lead, sizeof(int))) {
    // printf("pipe empty %d\n", getpid());
    close(p[0]);
    exit(0);
  }
  printf("prime %d\n", lead);
  pipe(p0);
  if (fork() == 0) {
    primes(p0);
  }
  else {
    close(p0[0]);
    while (read(p[0], &buf, sizeof(int))) {
      // printf("%d recv %d\n",getpid(),buf);
      if (buf % lead != 0) 
        write(p0[1], &buf, sizeof(int));
    }
    close(p[0]);
    close(p0[1]);

    int status;
    wait(&status);
    
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int p[2];
  pipe(p);
  if (fork() == 0) {
    primes(p);
    exit(0);
  }

  else {
    close(p[0]);
    for (int i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(int));
    }
    close(p[1]);

    int status;
    wait(&status);
    

    exit(0);
  }
}
