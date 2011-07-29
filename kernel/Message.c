#include "Message.h"
#include "Slab.h"
#include "Util.h"

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
}

void Message_Send(struct Connection *connection, void *sendBuf, int sendSize, void *replyBuf, int replySize)
{
	struct Channel *channel = connection->channel;
	struct Task *task = channel->task;
	struct Message message;

	message.connection = connection;
	message.sendBuf = sendBuf;
	message.sendSize = sendSize;
	message.replyBuf = replyBuf;
	message.replySize = replySize;
	LIST_ENTRY_CLEAR(message.list);
	LIST_ADD_TAIL(channel->waiters, message.list);

	if(task->state == TaskStateReceiveBlock) {
		Sched_AddHead(task);
	}

	Current->state = TaskStateSendBlock;
	Sched_RunNext();
}

struct Message *Message_Receive(struct Channel *channel, void *recvBuf, int recvSize)
{
	struct Message *message;
	int size;

	if(LIST_SIZE(channel->waiters) == 0) {
		Current->state = TaskStateReceiveBlock;
		Sched_RunNext();
	}

	message = LIST_HEAD(channel->waiters, struct Message, list);
	LIST_REMOVE(channel->waiters, message->list);

	memcpy(recvBuf, message->sendBuf, min(recvSize, message->sendSize));

	message->connection->task->state = TaskStateReplyBlock;
	return message;
}

void Message_Reply(struct Message *message, void *replyBuf, int replySize)
{
	struct Task *task = message->connection->task;

	memcpy(message->replyBuf, replyBuf, min(message->replySize, replySize));
	Sched_AddHead(task);
}

void Message_Init()
{
	Slab_Init(&channelSlab, sizeof(struct Channel));
	Slab_Init(&connectionSlab, sizeof(struct Connection));
}