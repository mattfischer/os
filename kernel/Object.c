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

static void translateObjects(struct Process *sourceProcess, struct Process *destProcess, char *buffer, int objectOffset, int numObjects, struct ObjectTranslate translateCache[])
{
	int i;

	for(i=0; i<numObjects; i++) {
		unsigned int *ptr;
		struct Object *object;

		if(translateCache[i].target != 0xffffffff) {
			*ptr = translateCache[i].target;
			continue;
		}

		ptr = (int*)(buffer + objectOffset) + i;

		if(sourceProcess == NULL) {
			object = (struct Object*)translateCache[i].source;
		} else {
			object = sourceProcess->objects[translateCache[i].source];
		}

		if(destProcess == NULL) {
			*ptr = (unsigned int)object;
		} else {
			*ptr = Process_RefObject(destProcess, object);
		}

		translateCache[i].target = *ptr;
	}
}

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Message message;
	struct Task *task;
	int i;

	message.sender = Current;
	message.sendMsg = *sendMsg;

	if(replyMsg == NULL) {
		memset(&message.replyMsg, 0, sizeof(message.replyMsg));
	} else {
		message.replyMsg = *replyMsg;
	}
	memset(message.translateCache, 0xff, sizeof(message.translateCache));

	for(i=0; i<sendMsg->objectsSize; i++) {
		message.translateCache[i].source = ((unsigned int*)((char*)sendMsg->body + sendMsg->objectsOffset))[i];
	}

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

	if(replyMsg != NULL) {
		*replyMsg = message.replyMsg;
		translateObjects(message.receiver->process, Current->process, replyMsg->body, replyMsg->objectsOffset, replyMsg->objectsSize, message.translateCache);
	}

	return message.ret;
}

struct Message *Object_ReceiveMessage(struct Object *object, struct MessageHeader *recvMsg)
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

	message->receiver = Current;

	recvMsg->size = min(message->sendMsg.size, recvMsg->size);
	recvMsg->objectsOffset = message->sendMsg.objectsOffset;
	recvMsg->objectsSize = message->sendMsg.objectsSize;

	AddressSpace_CopyFrom(message->sender->process->addressSpace, recvMsg->body, message->sendMsg.body, recvMsg->size);
	translateObjects(message->sender->process, Current->process, recvMsg->body, recvMsg->objectsOffset, recvMsg->objectsSize, message->translateCache);

	message->sender->state = TaskStateReplyBlock;
	return message;
}

int Object_ReplyMessage(struct Message *message, int ret, struct MessageHeader *replyMsg)
{
	struct Task *sender = message->sender;
	struct MessageHeader replyMsgLocal;
	struct AddressSpace *addressSpace;
	int i;

	if(replyMsg != NULL) {
		addressSpace = sender->process->addressSpace;
		message->replyMsg.size = min(message->replyMsg.size, replyMsg->size);
		message->replyMsg.objectsOffset = replyMsg->objectsOffset;
		message->replyMsg.objectsSize = replyMsg->objectsSize;
		memset(message->translateCache, 0xff, sizeof(message->translateCache));

		for(i=0; i<replyMsg->objectsSize; i++) {
			message->translateCache[i].source = ((unsigned int*)((char*)replyMsg->body + replyMsg->objectsOffset))[i];
		}

		AddressSpace_CopyTo(addressSpace, message->replyMsg.body, replyMsg->body, message->replyMsg.size);
	}

	message->ret = ret;

	Sched_Add(Current);
	Sched_SwitchTo(sender);

	return 0;
}
