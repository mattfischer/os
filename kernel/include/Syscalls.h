#ifndef SYSCALLS_H
#define SYSCALLS_H

enum Syscall {
	SyscallYield,
	SyscallSendMessage,
	SyscallReceiveMessage,
	SyscallReadMessage,
	SyscallReplyMessage,
	SyscallCreateObject,
	SyscallReleaseObject,
	SyscallGetProcessManager
};

#endif