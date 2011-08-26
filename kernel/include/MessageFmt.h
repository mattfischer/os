#ifndef MESSAGE_FMT_H
#define MESSAGE_FMT_H

#define MESSAGE_MAX_OBJECTS 4

struct BufferSegment {
	void *buffer;
	int size;
};

struct MessageHeader {
	struct BufferSegment *segments;
	int numSegments;
	int objectsOffset;
	int objectsSize;
};

struct MessageInfo {
	void *targetData;
};

struct Event {
	unsigned type;
	void *targetData;
	unsigned value;
};

enum SysEvent {
	SysEventObjectRef,
	SysEventObjectUnref
};

#endif