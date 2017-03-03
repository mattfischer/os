#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>
#include <kernel/include/IOFmt.h>

enum ProcessMsgType {
	ProcessMapPhys,
	ProcessSbrk,
	ProcessKill,
	ProcessWait
};

struct ProcessMsgMapPhys {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
};

struct ProcessMsgSbrk {
	unsigned int increment;
};

union ProcessMsg {
	struct {
		enum ProcessMsgType type;
		union {
			struct ProcessMsgMapPhys mapPhys;
			struct ProcessMsgSbrk sbrk;
		} u;
	} msg;
	struct Event event;
};

#endif