#include <System.h>
#include <Object.h>
#include <Kernel.h>

#include <stdlib.h>
extern int main(int argc, char *argv[]);

void _start()
{
  int argc = 0;
  char *argv[0];

  int ret = main(argc, argv);
  _exit(ret);
}