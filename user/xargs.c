#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void copy_arg(char *arg_d[], char *arg_s[], int n) {
  for (int i = 0; i < n; i ++) {
    strcpy(arg_d[i],arg_s[i]);
    // printf("arg: %s\n",arg_s[i]);
  }
}

int main(int argc, char *argv[]) {
  int max_arg = 32;
  char *arg_buf[32];
  for (int i = 0; i < 32; i ++) {
    arg_buf[i] = (char *)malloc(32);
  }
  int arg_cnt;

  // printf("allc finsh\n");
  if (argc < 2) {
    printf("xargs: need at least 1 arg\n");
    exit(0);
  }
  
  if (strcmp(argv[1], "-n") != 0) {
    copy_arg(arg_buf, argv + 1, argc - 1);
    arg_cnt = argc - 1;
  }

  else {
    if (argc < 4 || atoi(argv[2]) < 1) {
      printf("xargs: -n need an int arg > 0\n");
      exit(0);
    }

    max_arg = atoi(argv[2]);
    copy_arg(arg_buf, argv + 3, argc - 3);
    arg_cnt = argc - 3;
  }

  int end = 0;
  while (!end) {
    int arg_add = 0;
    char *arg_next;
    while (arg_add < max_arg) {
      arg_next = arg_buf[arg_cnt + arg_add];
      if (gets(arg_next, 32)[0] == '\0') {
        end = 1;
        break;
      }
      arg_next[strlen(arg_next) - 1] = '\0';
      if (strlen(arg_next) == 0) continue;
      // printf("arg_add: %s\n", arg_next);
      arg_add ++; 
      // printf("arg_sum: %d\n", arg_add);
    }
    
    // printf("arg_sum: %d\n", arg_cnt + arg_add);
    if (arg_add > 0) {
      arg_next = arg_buf[arg_cnt + arg_add];
      arg_buf[arg_cnt + arg_add] = 0;
      
      if (fork() == 0) {
        // printf("exec: %s\n", arg_buf[0]);
        // for (int i = 0; i < 32; i ++) {
        //   if (strcmp(arg_buf[i], "\0") == 0) break;
        //   else printf("exec arg: %s\n", arg_buf[i]);
        // }
        exec(arg_buf[0], arg_buf);
      }
      
      int empty;
      wait(&empty);

      arg_buf[arg_cnt + arg_add] = arg_next;
    }

  }

  for (int i = 0; i < 32; i ++) {
    free(arg_buf[i]);
  }

  exit(0);

}