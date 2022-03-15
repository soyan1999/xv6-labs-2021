#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  // static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  // if(strlen(p) >= DIRSIZ)
    return p;
  // memmove(buf, p, strlen(p));
  // memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  // return buf;
}

void
find(char *path, char *name)
{
  // printf("path: %s\nname: %s\n",path,name);
  
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if(stat(path, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    return;
  }

  if (strcmp(name, fmtname(path)) == 0) printf("%s\n", path);

  if (st.type == T_DIR){
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      return;
    }

    if((fd = open(path, 0)) < 0){
      fprintf(2, "find: cannot open %s\n", path);
      return;
    }

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    char *dn1 = ".", *dn2 = "..", *fmt_de = de.name;
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0 || strcmp(dn1, fmt_de) == 0 || strcmp(dn2, fmt_de) == 0)
        continue;
      strcpy(p, fmt_de);

      find(buf, name);
    }
    close(fd);
  }
}

int
main(int argc, char *argv[])
{

  if(argc > 2){
    find(argv[1], argv[2]);
    exit(0);
  }
  else printf("find need at least 2 arg\n");
  exit(0);
}
