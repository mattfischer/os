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

static void readBuffer(struct Process *destProcess, void *dest, struct Process *srcProcess, struct MessageHeader *src, int offset, int size, int translateCache[])
{
	int i;

	AddressSpace_Memcpy(destProcess->addressSpace, dest, srcProcess->addressSpace, (char*)src->body + offset, size);

	for(i=0; i<src->objectsSize; i++) {
		int *s, *d;
		int objOffset;
		int obj;

		objOffset = src->objectsOffset + i * sizeof(int);
		if(objOffset < offset || objOffset >= offset + size) {
			continue;
		}

		s = (int*)PADDR_TO_VADDR(PageTable_TranslateVAddr(srcProcess->addressSpace->pageTable, (char*)src->body + objOffset));
		d = (int*)PADDR_TO_VADDR(PageTable_TranslateVAddr(destProcess->addressSpace->pageTable, dest + objOffset - offset));
		obj = *s;

		if(translateCache[i] == INVALID_OBJECT) {
			translateCache[i] = Process_DupObjectRef(destProcess, srcProcess, obj);
		}

		*d = translateCache[i];
	}
}

static void copyBuffer(struct Process *destProcess, struct MessageHeader *dest, struct Process *srcProcess, struct MessageHeader *src, int translateCache[])
{
	int size;

	size = min(src->size, dest->size);

	dest->objectsOffset = src->objectsOffset;
	dest->objectsSize = src->objectsSize;
	dest->size = size;

	readBuffer(destProcess, dest->body, srcProcess, src, 0, size, translateCache);
}

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Message message;
	struct Task *task;
	int i;

	message.sender = Current;
	message.sendMsg = *sendMsg;
	message.replyMsg = *replyMsg;

	for(i=0; i<MESSAGE_MAX_OBJECTS; i++) {
		message.translateCache[i] = INVALID_OBJECT;
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

	copyBuffer(Current->process, recvMsg, message->sender->process, &message->sendMsg, message->translateCache);

	message->receiver = Current;
	message->sender->state = TaskStateReplyBlock;

	return message;
}

int Object_ReadMessage(struct Message *message, void *buffer, int offset, int size)
{
	int s;

	s = min(message->sendMsg.size - offset, size);
	if(s < 0) {
		return 0;
	}

	readBuffer(Current->process, buffer, message->sender->process, &message->sendMsg, offset, s, message->translateCache);

	return s;
}

int Object_ReplyMessage(struct Message *message, int ret, struct MessageHeader *replyMsg)
{
	int i;
	int translateCache[MESSAGE_MAX_OBJECTS];

	for(i=0; i<MESSAGE_MAX_OBJECTS; i++) {
		translateCache[i] = INVALID_OBJECT;
	}

	copyBuffer(message->sender->process, &message->replyMsg, Current->process, replyMsg, translateCache);
	message->ret = ret;

	Sched_Add(Current);
	Sched_SwitchTo(message->sender);

	return 0;
}

int CreateObject()
{
	struct Object *object = Object_Create();
	return Process_RefObject(Current->process, object);
}

void ReleaseObject(int obj)
{
	Process_UnrefObject(Current->process, obj);
}