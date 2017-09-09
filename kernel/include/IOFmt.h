#ifndef IO_FMT_H
#define IO_FMT_H

#include <kernel/include/MessageFmt.h>

enum IOMsgType {
	IOMsgTypeWrite,
	IOMsgTypeRead,
	IOMsgTypeSeek,
	IOMsgTypeReadDir
};

struct IOMsgReadWriteHdr {
	int size;
};

struct IOMsgSeek {
	int pointer;
};

struct IOMsgReadDirRet {
	char name[32];
};

struct IOMsg {
	union {
		struct {
			enum IOMsgType type;
			union {
				struct IOMsgReadWriteHdr rw;
				struct IOMsgSeek seek;
			};
		};
		struct Event event;
	};
};

#endif