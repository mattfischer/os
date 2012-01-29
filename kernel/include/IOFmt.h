#ifndef IO_FMT_H
#define IO_FMT_H

#include <kernel/include/MessageFmt.h>

enum IOMsgType {
	IOMsgTypeWrite,
	IOMsgTypeRead,
	IOMsgTypeSeek
};

struct IOMsgReadWriteHdr {
	int size;
};

struct IOMsgSeek {
	int pointer;
};

union IOMsg {
	struct {
		enum IOMsgType type;
		union {
			struct IOMsgReadWriteHdr rw;
			struct IOMsgSeek seek;
		} u;
	} msg;
	struct Event event;
};

#endif