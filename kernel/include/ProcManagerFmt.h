#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>
#include <kernel/include/IOFmt.h>

enum ProcManagerMsgType {
	ProcManagerMapPhys,
	ProcManagerSbrk,
	ProcManagerSpawnProcess,
	ProcManagerSubInt,
	ProcManagerUnsubInt,
	ProcManagerAckInt,
	ProcManagerKill,
	ProcManagerWait,
	ProcManagerReadLog
};

struct ProcManagerMsgMapPhys {
	unsigned int vaddr;
	unsigned int paddr;
	unsigned int size;
};

struct ProcManagerMsgSbrk {
	unsigned int increment;
};

#define PROC_MANAGER_CMDLINE_LEN 64
struct ProcManagerMsgSpawnProcess {
	char cmdline[PROC_MANAGER_CMDLINE_LEN];
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

struct ProcManagerReadLog {
	int offset;
	int size;
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
			struct ProcManagerReadLog readLog;
		} u;
	} msg;
	struct Event event;
};

#endif