#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>
#include <kernel/include/IOFmt.h>

enum ProcessMsgType {
	ProcessMapPhys,
	ProcessMap,
	ProcessExpandMap,
	ProcessKill,
	ProcessWait
};

struct ProcessMsgMapPhys {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
};

struct ProcessMsgMap {
	unsigned int vaddr;
	unsigned int size;
};

struct ProcessMsg {
	union {
		struct {
			enum ProcessMsgType type;
			union {
				struct ProcessMsgMapPhys mapPhys;
				struct ProcessMsgMap map;
			};
		};
		struct Event event;
	};
};

#endif