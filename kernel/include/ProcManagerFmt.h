#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

enum ProcManagerMsgType {
	ProcManagerNameLookup,
	ProcManagerNameSet,
	ProcManagerMapPhys,
	ProcManagerSbrk
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

struct ProcManagerMsg {
	enum ProcManagerMsgType type;
	union {
		struct ProcManagerMsgNameLookup lookup;
		struct ProcManagerMsgNameSet set;
		struct ProcManagerMsgMapPhys mapPhys;
		struct ProcManagerMsgSbrk sbrk;
	} u;
};

#endif