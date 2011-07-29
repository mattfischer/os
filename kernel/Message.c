#include "Message.h"
#include "Slab.h"

struct SlabAllocator channelSlab;
struct SlabAllocator connectionSlab;

struct Channel *ChannelCreate(struct Task *task)
{
	int i;
	struct Channel *channel;

	channel = (struct Channel*)SlabAllocate(&channelSlab);
	channel->task = task;
	LIST_INIT(channel->waiters);
}

struct Connection *ConnectionCreate(struct Task *task, struct Channel *channel)
{
	int i;
	struct Connection *connection;

	connection = (struct Connection*)SlabAllocate(&connectionSlab);
	connection->task = task;
	connection->channel = channel;
	LIST_ENTRY_CLEAR(connection->list);
}

void MessageSend(struct Connection *connection)
{
	struct Channel *channel = connection->channel;
	struct Task *task = channel->task;

	LIST_ADD_TAIL(channel->waiters, connection->list);

	if(task->state == TaskStateReceiveBlock) {
		TaskAdd(task);
	}

	Current->state = TaskStateSendBlock;
	Schedule();
}

struct Connection *MessageReceive(struct Channel *channel)
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

void MessageReply(struct Connection *connection)
{
	struct Task *task = connection->task;

	TaskAdd(task);
}

void MessageInit()
{
	SlabInit(&channelSlab, sizeof(struct Channel));
	SlabInit(&connectionSlab, sizeof(struct Connection));
}