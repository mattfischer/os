#ifndef SYSCALLS_H
#define SYSCALLS_H

enum KernelObject {
	KernelObjectProcManager,
	KernelObjectNameServer
};

enum Syscall {
	SyscallYield,
	SyscallObjectCreate,
	SyscallObjectRelease,
	SyscallObjectSend,
	SyscallObjectPost,
	SyscallObjectReceive,
	SyscallMessageRead,
	SyscallMessageReply,
	SyscallMessageInfo,
	SyscallKernelGetNameServer,
	SyscallKernelSetNameServer
};

#endif