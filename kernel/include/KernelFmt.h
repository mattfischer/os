#ifndef KERNEL_FMT_H
#define KERNEL_FMT_H

#include <kernel/include/MessageFmt.h>
#include <kernel/include/IOFmt.h>

enum KernelMsgType {
	KernelSpawnProcess,
	KernelSubInt,
	KernelUnmaskInt,
	KernelReadLog
};

#define KERNEL_CMDLINE_LEN 64
struct KernelMsgSpawnProcess {
	char cmdline[KERNEL_CMDLINE_LEN];
	int stdinObject;
	int stdoutObject;
	int stderrObject;
	int nameserverObject;
};

struct KernelMsgSubInt {
	unsigned irq;
	int object;
	unsigned type;
	unsigned value;
};

struct KernelMsgUnmaskInt {
	int irq;
};

struct KernelMsgReadLog {
	int offset;
	int size;
};

union KernelMsg {
	struct {
		enum KernelMsgType type;
		union {
			struct KernelMsgSpawnProcess spawn;
			struct KernelMsgSubInt subInt;
			struct KernelMsgUnmaskInt unmaskInt;
			struct KernelMsgReadLog readLog;
		} u;
	} msg;
	struct Event event;
};

#endif