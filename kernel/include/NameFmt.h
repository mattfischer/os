#ifndef NAME_FMT_H
#define NAME_FMT_H

#include <kernel/include/MessageFmt.h>

enum NameMsgType {
	NameMsgTypeSet,
	NameMsgTypeLookup,
	NameMsgTypeOpen
};

struct NameMsgSetHdr {
	int obj;
	char name[32];
};

struct NameMsgLookupHdr {
	char name[32];
};

union NameMsg {
	struct {
		enum NameMsgType type;
		union {
			struct NameMsgSetHdr set;
			struct NameMsgLookupHdr lookup;
		} u;
	} msg;
	struct Event event;
};

enum NameEntryMsgType {
	NameEntryMsgTypeOpen
};

struct NameEntryMsgOpen {
	char name[32];
};

union NameEntryMsg {
	struct {
		enum NameEntryMsgType type;
		union {
			struct NameEntryMsgOpen open;
		} u;
	} msg;
	struct Event event;
};

#endif