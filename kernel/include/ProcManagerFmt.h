#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

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

struct ProcManagerMsg {
	enum ProcManagerMsgType type;
	union {
		struct ProcManagerMsgMapPhys mapPhys;
		struct ProcManagerMsgSbrk sbrk;
		struct ProcManagerMsgSpawnProcess spawn;
	} u;
};

#endif