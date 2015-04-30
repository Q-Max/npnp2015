#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


int main( int argc, char *argv[] )
{

  FILE *fp;
  char path[1035];
  char cwd[1024];
   if (getcwd(cwd, sizeof(cwd)) != NULL)
       fprintf(stdout, "Current working dir: %s\n", cwd);
   else
       perror("getcwd() error");
  struct stat st = {0};
  if(stat("/home/q-max/np/test", &st)==-1){
    mkdir("/home/q-max/np/test", 0700);
  }

  /* Open the command for reading. */
  fp = popen("/bin/ls /home/q-max/np/ -F", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit(1);
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    printf("%s", path);
  }

  /* close */
  pclose(fp);

  return 0;
}
/*
    char * argv[] = {"ls", "-al", "/etc/passwd", (char*)};
    execv("/bin/ls", argv);
    */