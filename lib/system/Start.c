#include <System.h>
#include <Object.h>

#include <stdlib.h>
#include <unistd.h>
extern int main(int argc, char *argv[]);

void _start(void *cmdline)
{
  int argc;
  char *argv[16];
  char *c;

  // Construct argv out of the passed-in command line
  for(argc=0, c = cmdline; *c != '\0'; argc++) {
    argv[argc] = c;
    while(*c != '\0') c++;
    c++;
  }

  int ret = main(argc, argv);
  _exit(ret);
}