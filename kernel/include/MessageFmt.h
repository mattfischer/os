#ifndef MESSAGE_FMT_H
#define MESSAGE_FMT_H

struct MessageHeader {
	int size;
	void *body;
	int objectsOffset;
	int objectsSize;
};

#endif