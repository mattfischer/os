#ifndef IO_FMT_H
#define IO_FMT_H

#include <kernel/include/MessageFmt.h>

enum IOMsgType {
	IOMsgTypeWrite
};

struct IOMsgWriteHdr {
	int size;
};

union IOMsg {
	struct {
		enum IOMsgType type;
		union {
			struct IOMsgWriteHdr write;
		} u;
	} msg;
	struct Event event;
};

#endif