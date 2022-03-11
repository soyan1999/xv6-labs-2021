#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int sec;

  if(argc < 2){
    fprintf(2, "Error: need an int arg\n");
    exit(1);
  }

  sec = atoi(argv[1]);
  sleep(sec);

  exit(0);
}
