#ifndef MESSAGE_H
#define MESSAGE_H

#include "Sched.h"
#include "List.h"

struct Channel {
	struct Task *task;
	LIST(struct Message) waiters;
};

struct Connection {
	struct Task *task;
	struct Channel *channel;
};

struct Message {
	struct Connection *connection;
	void *sendBuf;
	int sendSize;
	void *replyBuf;
	int replySize;
	struct ListEntry list;
};

struct Channel *Channel_Create(struct Task *task);
struct Connection *Connection_Create(struct Task *task, struct Channel *channel);

void Message_Send(struct Connection *connection, void *sendBuf, int sendSize, void *replyBuf, int replySize);
struct Message *Message_Receive(struct Channel *channel, void *recvBuf, int recvSize);
void Message_Reply(struct Message *message, void *replyBuf, int replySize);

void Message_Init();

#endif