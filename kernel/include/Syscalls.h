#ifndef SYSCALLS_H
#define SYSCALLS_H

enum KernelObject {
	KernelObjectProcManager,
	KernelObjectNameServer,
	KernelObjectCount
};

enum Syscall {
	SyscallYield,
	SyscallSendMessage,
	SyscallReceiveMessage,
	SyscallReadMessage,
	SyscallReplyMessage,
	SyscallCreateObject,
	SyscallReleaseObject,
	SyscallGetObject,
	SyscallSetObject
};

#endif