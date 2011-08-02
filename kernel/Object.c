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

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Message message;
	struct Task *task;

	message.sender = Current;
	message.sendMsg = *sendMsg;
	message.replyMsg = *replyMsg;
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

struct Message *Object_ReceiveMessage(struct Object *object, struct MessageHeader *recvMsg)
{
	struct Message *message;
	struct AddressSpace *addressSpace;
	int size;

	if(LIST_EMPTY(object->messages)) {
		LIST_ADD_TAIL(object->receivers, Current->list);
		Current->state = TaskStateReceiveBlock;
		Sched_RunNext();
	}

	message = LIST_HEAD(object->messages, struct Message, list);
	LIST_REMOVE(object->messages, message->list);

	addressSpace = message->sender->process->addressSpace;
	recvMsg->size = min(message->sendMsg.size, recvMsg->size);
	AddressSpace_CopyFrom(addressSpace, recvMsg->body, message->sendMsg.body, recvMsg->size);

	message->sender->state = TaskStateReplyBlock;
	return message;
}

int Object_ReplyMessage(struct Message *message, struct MessageHeader *replyMsg)
{
	struct Task *sender = message->sender;
	struct MessageHeader replyMsgLocal;
	struct AddressSpace *addressSpace;

	addressSpace = sender->process->addressSpace;
	message->replyMsg.size = min(message->replyMsg.size, replyMsg->size);
	AddressSpace_CopyTo(addressSpace, message->replyMsg.body, replyMsg->body, message->replyMsg.size);

	Sched_Add(Current);
	Sched_SwitchTo(sender);

	return 0;
}
