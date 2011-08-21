#include "include/System.h"

extern int main(int argc, char *argv[]);

int __ProcessManager;

void _start()
{
  int argc = 0;
  char *argv[0];

  __ProcessManager = GetProcessManager();
  int ret = main(argc, argv);
}