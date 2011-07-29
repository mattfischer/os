#include "Message.h"
#include "Slab.h"

struct SlabAllocator channelSlab;
struct SlabAllocator connectionSlab;

struct Channel *ChannelCreate(struct Task *task)
{
	int i;
	struct Channel *channel;

	channel = (struct Channel*)SlabAllocate(&channelSlab);
	channel->waiters = NULL;
	channel->waitersTail = NULL;
	channel->task = task;
	for(i=0; i<16; i++) {
		if(task->channels[i] == NULL) {
			task->channels[i] = channel;
			break;
		}
	}
}

struct Connection *ConnectionCreate(struct Task *task, struct Channel *channel)
{
	int i;
	struct Connection *connection;

	connection = (struct Connection*)SlabAllocate(&connectionSlab);
	connection->task = task;
	connection->channel = channel;
	connection->next = NULL;
	for(i=0; i<16; i++) {
		if(task->connections[i] == NULL) {
			task->connections[i] = connection;
			break;
		}
	}
}

void MessageSend(struct Connection *connection)
{
	struct Channel *channel = connection->channel;
	struct Task *task = channel->task;

	if(channel->waitersTail == NULL) {
		channel->waiters = connection;
	} else {
		channel->waitersTail->next = connection;
	}
	connection->next = NULL;
	channel->waitersTail = connection;

	if(task->state == TaskStateReceiveBlock) {
		TaskAdd(task);
	}

	Current->state = TaskStateSendBlock;
	Schedule();
}

struct Connection *MessageReceive(struct Channel *channel)
{
	struct Connection *connection;

	if(channel->waiters == NULL) {
		Current->state = TaskStateReceiveBlock;
		Schedule();
	}

	connection = channel->waiters;
	channel->waiters = connection->next;
	if(channel->waiters == NULL) {
		channel->waitersTail = NULL;
	}

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