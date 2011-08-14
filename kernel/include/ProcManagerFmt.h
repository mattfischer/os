#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

enum ProcManagerMsgType {
	ProcManagerNameLookup,
	ProcManagerNameSet,
	ProcManagerMapPhys,
	ProcManagerSbrk,
	ProcManagerSpawnProcess
};

#define PROC_MANAGER_MSG_NAME_MAX_LEN 20

struct ProcManagerMsgNameLookup {
	char name[PROC_MANAGER_MSG_NAME_MAX_LEN];
};

struct ProcManagerMsgNameSet {
	char name[PROC_MANAGER_MSG_NAME_MAX_LEN];
	int obj;
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
		struct ProcManagerMsgNameLookup lookup;
		struct ProcManagerMsgNameSet set;
		struct ProcManagerMsgMapPhys mapPhys;
		struct ProcManagerMsgSbrk sbrk;
		struct ProcManagerMsgSpawnProcess spawn;
	} u;
};

#endif