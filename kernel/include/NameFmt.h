#ifndef NAME_FMT_H
#define NAME_FMT_H

enum NameMsgType {
	NameMsgTypeSet,
	NameMsgTypeLookup
};

struct NameMsgSetHdr {
	int obj;
	char name[32];
};

struct NameMsgLookupHdr {
	char name[32];
};

struct NameMsg {
	enum NameMsgType type;
	union {
		struct NameMsgSetHdr set;
		struct NameMsgLookupHdr lookup;
	} u;
};

#endif