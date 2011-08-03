#ifndef MESSAGE_FMT_H
#define MESSAGE_FMT_H

#define MESSAGE_MAX_OBJECTS 4

struct MessageHeader {
	int size;
	void *body;
	int objectsOffset;
	int objectsSize;
};

#endif