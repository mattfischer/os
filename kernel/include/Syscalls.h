#ifndef SYSCALLS_H
#define SYSCALLS_H

enum Syscall {
	SyscallYield,
	SyscallObjectCreate,
	SyscallObjectRelease,
	SyscallObjectSend,
	SyscallObjectPost,
	SyscallMessageRead,
	SyscallMessageReply,
	SyscallChannelCreate,
	SyscallChannelDestroy,
	SyscallChannelReceive
};

#endif