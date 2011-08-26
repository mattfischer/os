#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>

enum ProcManagerMsgType {
	ProcManagerMapPhys,
	ProcManagerSbrk,
	ProcManagerSpawnProcess
};

struct ProcManagerMsgMapPhys {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
};

struct ProcManagerMsgSbrk {
	unsigned int increment;
};

struct ProcManagerMsgSpawnProcess {
	char name[20];
	int stdinObject;
	int stdoutObject;
	int stderrObject;
};

union ProcManagerMsg {
	struct {
		enum ProcManagerMsgType type;
		union {
			struct ProcManagerMsgMapPhys mapPhys;
			struct ProcManagerMsgSbrk sbrk;
			struct ProcManagerMsgSpawnProcess spawn;
		} u;
	} msg;
	struct Event event;
};

#endif