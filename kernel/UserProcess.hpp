#ifndef USER_PROCESS_H
#define USER_PROCESS_H

#include "Process.hpp"

class UserProcess {
public:
	static void start(Process *process, const char *cmdline, int stdinObject, int stdoutObject, int stderrObject, int kernelObject, int processObject, int nameserverObject);

};

#endif