#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

enum ProcManagerMsgType {
	ProcManagerNameLookup,
	ProcManagerNameSet,
	ProcManagerMapPhys
};

#define PROC_MANAGER_MSG_NAME_MAX_LEN 20

struct ProcManagerMsgNameLookup {
	char name[PROC_MANAGER_MSG_NAME_MAX_LEN];
};

struct ProcManagerMsgNameLookupReply {
	unsigned int obj;
};

struct ProcManagerMsgNameSet {
	char name[PROC_MANAGER_MSG_NAME_MAX_LEN];
	unsigned int obj;
};

struct ProcManagerMsgMapPhys {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
};

struct ProcManagerMsg {
	enum ProcManagerMsgType type;
	union {
		struct ProcManagerMsgNameLookup lookup;
		struct ProcManagerMsgNameSet set;
		struct ProcManagerMsgMapPhys mapPhys;
	} u;
};

#endif