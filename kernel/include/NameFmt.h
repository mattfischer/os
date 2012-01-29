#ifndef NAME_FMT_H
#define NAME_FMT_H

#include <kernel/include/MessageFmt.h>

enum NameMsgType {
	NameMsgTypeSet,
	NameMsgTypeLookup,
	NameMsgTypeOpen,
	NameMsgTypeWait
};

struct NameMsgSetHdr {
	int obj;
	char name[32];
};

struct NameMsgLookupHdr {
	char name[32];
};

struct NameMsgOpen {
	char name[32];
};

struct NameMsgWait {
	char name[32];
};

union NameMsg {
	struct {
		enum NameMsgType type;
		union {
			struct NameMsgSetHdr set;
			struct NameMsgLookupHdr lookup;
			struct NameMsgOpen open;
			struct NameMsgWait wait;
		} u;
	} msg;
	struct Event event;
};

#endif