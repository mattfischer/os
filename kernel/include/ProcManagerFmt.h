#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>

enum ProcManagerMsgType {
	ProcManagerMapPhys,
	ProcManagerSbrk,
	ProcManagerSpawnProcess,
	ProcManagerSubInt,
	ProcManagerUnsubInt,
	ProcManagerAckInt,
	ProcManagerKill,
	ProcManagerWait
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

struct ProcManagerMsgSubInt {
	unsigned irq;
	int object;
	unsigned type;
	unsigned value;
};

struct ProcManagerMsgUnsubInt {
	int sub;
};

struct ProcManagerMsgAckInt {
	int sub;
};

union ProcManagerMsg {
	struct {
		enum ProcManagerMsgType type;
		union {
			struct ProcManagerMsgMapPhys mapPhys;
			struct ProcManagerMsgSbrk sbrk;
			struct ProcManagerMsgSpawnProcess spawn;
			struct ProcManagerMsgSubInt subInt;
			struct ProcManagerMsgUnsubInt unsubInt;
			struct ProcManagerMsgAckInt ackInt;
		} u;
	} msg;
	struct Event event;
};

#endif