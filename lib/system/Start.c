#include "include/System.h"
#include "include/Object.h"

extern int main(int argc, char *argv[]);

int __ProcessManager;
int __NameServer = INVALID_OBJECT;

void _start()
{
  int argc = 0;
  char *argv[0];

  __ProcessManager = GetKernelObject(KernelObjectProcManager);

  int ret = main(argc, argv);
}