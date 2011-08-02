#include "Object.h"
#include "Slab.h"
#include "Sched.h"
#include "Util.h"

static struct SlabAllocator objectSlab;

struct Object *Object_Create()
{
	struct Object *object = (struct Object*)Slab_Allocate(&objectSlab);
	LIST_INIT(object->receivers);
	LIST_INIT(object->messages);

	return object;
}

void Object_Init()
{
	Slab_Init(&objectSlab, sizeof(struct Object));
}

int Object_SendMessage(struct Object *object, void *sendBuf, int sendSize, void *replyBuf, int replySize)
{
	struct Message message;
	struct Task *task;

	message.sender = Current;
	message.sendBuf = sendBuf;
	message.sendSize = sendSize;
	message.replyBuf = replyBuf;
	message.replySize = replySize;
	LIST_ENTRY_CLEAR(message.list);
	LIST_ADD_TAIL(object->messages, message.list);

	Current->state = TaskStateSendBlock;

	if(!LIST_EMPTY(object->receivers)) {
		task = LIST_HEAD(object->receivers, struct Task, list);
		LIST_REMOVE(object->receivers, task->list);
		Sched_SwitchTo(task);
	} else {
		Sched_RunNext();
	}

	return 0;
}

struct Message *Object_ReceiveMessage(struct Object *object, void *recvBuf, int recvSize)
{
	struct Message *message;
	int size;

	if(LIST_EMPTY(object->messages)) {
		LIST_ADD_TAIL(object->receivers, Current->list);
		Current->state = TaskStateReceiveBlock;
		Sched_RunNext();
	}

	message = LIST_HEAD(object->messages, struct Message, list);
	LIST_REMOVE(object->messages, message->list);

	AddressSpace_CopyFrom(message->sender->process->addressSpace, recvBuf, message->sendBuf, min(recvSize, message->sendSize));

	message->sender->state = TaskStateReplyBlock;
	return message;
}

int Object_ReplyMessage(struct Message *message, void *replyBuf, int replySize)
{
	struct Task *sender = message->sender;

	AddressSpace_CopyTo(sender->process->addressSpace, message->replyBuf, replyBuf, min(message->replySize, replySize));

	Sched_Add(Current);
	Sched_SwitchTo(sender);

	return 0;
}
