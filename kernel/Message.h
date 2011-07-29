#ifndef MESSAGE_H
#define MESSAGE_H

#include "Sched.h"
#include "List.h"

struct Channel {
	struct Task *task;
	struct List waiters;
};

struct Connection {
	struct Task *task;
	struct Channel *channel;
	struct ListEntry list;
};

struct Channel *ChannelCreate(struct Task *task);
struct Connection *ConnectionCreate(struct Task *task, struct Channel *channel);

void MessageSend(struct Connection *connection);
struct Connection *MessageReceive(struct Channel *channel);
void MessageReply(struct Connection *connection);

void MessageInit();

#endif