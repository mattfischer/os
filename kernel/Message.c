#include "Message.h"
#include "Slab.h"

struct SlabAllocator channelSlab;
struct SlabAllocator connectionSlab;

struct Channel *Channel_Create(struct Task *task)
{
	int i;
	struct Channel *channel;

	channel = (struct Channel*)Slab_Allocate(&channelSlab);
	channel->task = task;
	LIST_INIT(channel->waiters);
}

struct Connection *Connection_Create(struct Task *task, struct Channel *channel)
{
	int i;
	struct Connection *connection;

	connection = (struct Connection*)Slab_Allocate(&connectionSlab);
	connection->task = task;
	connection->channel = channel;
	LIST_ENTRY_CLEAR(connection->list);
}

void Message_Send(struct Connection *connection)
{
	struct Channel *channel = connection->channel;
	struct Task *task = channel->task;

	LIST_ADD_TAIL(channel->waiters, connection->list);

	if(task->state == TaskStateReceiveBlock) {
		Task_AddTail(task);
	}

	Current->state = TaskStateSendBlock;
	Schedule();
}

struct Connection *Message_Receive(struct Channel *channel)
{
	struct Connection *connection;

	if(LIST_SIZE(channel->waiters) == 0) {
		Current->state = TaskStateReceiveBlock;
		Schedule();
	}

	connection = LIST_HEAD(channel->waiters, struct Connection, list);
	LIST_REMOVE(channel->waiters, connection->list);

	connection->task->state = TaskStateReplyBlock;
	return connection;
}

void Message_Reply(struct Connection *connection)
{
	struct Task *task = connection->task;

	Task_AddTail(task);
}

void Message_Init()
{
	Slab_Init(&channelSlab, sizeof(struct Channel));
	Slab_Init(&connectionSlab, sizeof(struct Connection));
}