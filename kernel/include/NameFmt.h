#ifndef NAME_FMT_H
#define NAME_FMT_H

#include <kernel/include/MessageFmt.h>

enum NameMsgType {
	NameMsgTypeSet,
	NameMsgTypeLookup,
	NameMsgTypeOpen,
	NameMsgTypeOpenDir,
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

struct NameMsgOpenDir {
	char name[32];
};

struct NameMsgWait {
	char name[32];
};

struct NameMsg {
	union {
		struct {
			enum NameMsgType type;
			union {
				struct NameMsgSetHdr set;
				struct NameMsgLookupHdr lookup;
				struct NameMsgOpen open;
				struct NameMsgOpenDir openDir;
				struct NameMsgWait wait;
			};
		};
		struct Event event;
	};
};

#endif