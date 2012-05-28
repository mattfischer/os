#include <System.h>
#include <Object.h>
#include <Kernel.h>

#include <stdlib.h>
extern int main(int argc, char *argv[]);

int __ProcessManager;

void _start()
{
  int argc = 0;
  char *argv[0];

  __ProcessManager = Kernel_GetObject(KernelObjectProcManager);

  int ret = main(argc, argv);
  _exit(ret);
}