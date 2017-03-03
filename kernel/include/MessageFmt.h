#ifndef MESSAGE_FMT_H
#define MESSAGE_FMT_H

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

struct Event {
	unsigned type;
	unsigned value;
};

enum SysEvent {
	SysEventObjectClosed,
	SysEventLast
};

enum SysError {
	SysErrorSuccess = 0,
	SysErrorObjectDead = -1,
	SysErrorLast = -2
};

#endif