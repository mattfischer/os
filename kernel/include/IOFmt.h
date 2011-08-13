#ifndef IO_FMT_H
#define IO_FMT_H

enum IOMsgType {
	IOMsgTypeWrite,
	IOMsgTypeLast
};

struct IOMsgWrite {
	int size;
	char data[1];
};

struct IOMsg {
	enum IOMsgType type;
	union {
		struct IOMsgWrite write;
	} u;
};

#endif