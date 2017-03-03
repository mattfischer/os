#ifndef PROC_MANAGER_FMT_H
#define PROC_MANAGER_FMT_H

#include <kernel/include/MessageFmt.h>
#include <kernel/include/IOFmt.h>

enum ProcManagerMsgType {
	ProcManagerMapPhys,
	ProcManagerSbrk,
	ProcManagerSpawnProcess,
	ProcManagerSubInt,
	ProcManagerUnmaskInt,
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
	int nameserverObject;
};

struct ProcManagerMsgSubInt {
	unsigned irq;
	int object;
	unsigned type;
	unsigned value;
};

struct ProcManagerMsgUnmaskInt {
	int irq;
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
			struct ProcManagerMsgUnmaskInt unmaskInt;
			struct ProcManagerReadLog readLog;
		} u;
	} msg;
	struct Event event;
};

#endif