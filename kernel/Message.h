#ifndef MESSAGE_H
#define MESSAGE_H

#include "Sched.h"

struct Channel {
	struct Task *task;
	struct Connection *waiters;
	struct Connection *waitersTail;
};

struct Connection {
	struct Task *task;
	struct Channel *channel;
	struct Connection *next;
};

struct Channel *ChannelCreate(struct Task *task);
struct Connection *ConnectionCreate(struct Task *task, struct Channel *channel);

void MessageSend(struct Connection *connection);
struct Connection *MessageReceive(struct Channel *channel);
void MessageReply(struct Connection *connection);

void MessageInit();

#endif