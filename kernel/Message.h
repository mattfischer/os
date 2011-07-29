#ifndef MESSAGE_H
#define MESSAGE_H

#include "Sched.h"
#include "List.h"

struct Channel {
	struct Task *task;
	LIST(struct Connection) waiters;
};

struct Connection {
	struct Task *task;
	struct Channel *channel;
	struct ListEntry list;
};

struct Channel *Channel_Create(struct Task *task);
struct Connection *Connection_Create(struct Task *task, struct Channel *channel);

void Message_Send(struct Connection *connection);
struct Connection *Message_Receive(struct Channel *channel);
void Message_Reply(struct Connection *connection);

void Message_Init();

#endif