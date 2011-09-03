#include <System.h>
#include <Object.h>
#include <Kernel.h>

extern int main(int argc, char *argv[]);

int __ProcessManager;
int __NameServer = OBJECT_INVALID;

void _start()
{
  int argc = 0;
  char *argv[0];

  __ProcessManager = Kernel_GetObject(KernelObjectProcManager);

  int ret = main(argc, argv);
}