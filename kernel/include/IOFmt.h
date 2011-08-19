#ifndef IO_FMT_H
#define IO_FMT_H

enum IOMsgType {
	IOMsgTypeWrite,
	IOMsgTypeLast
};

struct IOMsgWriteHdr {
	int size;
};

struct IOMsg {
	enum IOMsgType type;
	union {
		struct IOMsgWriteHdr write;
	} u;
};

#endif