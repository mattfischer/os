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

static void copyBuffer(struct Process *destProcess, struct MessageHeader *dest, struct Process *srcProcess, struct MessageHeader *src, int translateCache[])
{
	int i;
	int size;

	size = min(src->size, dest->size);
	AddressSpace_Memcpy(destProcess->addressSpace, dest->body, srcProcess->addressSpace, src->body, size);

	dest->objectsOffset = src->objectsOffset;
	dest->objectsSize = src->objectsSize;
	dest->size = size;

	if(src->objectsSize > 0) {
		int *srcObjects = (int*)((char*)src->body + src->objectsOffset);
		int *destObjects = (int*)((char*)dest->body + src->objectsOffset);

		for(i=0; i<src->objectsSize; i++) {
			int *s = (int*)PADDR_TO_VADDR(PageTable_TranslateVAddr(srcProcess->addressSpace->pageTable, srcObjects + i));
			int *d = (int*)PADDR_TO_VADDR(PageTable_TranslateVAddr(destProcess->addressSpace->pageTable, destObjects + i));
			int obj = *s;
			if(translateCache[i] == INVALID_OBJECT) {
				translateCache[i] = Process_DupObjectRef(destProcess, srcProcess, obj);
			}

			*d = translateCache[i];
		}
	}
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
	size = min(message->sendMsg.size, size);
	AddressSpace_Memcpy(Current->process->addressSpace, buffer, message->sender->process->addressSpace, (char*)message->sendMsg.body + offset, size);

	return size;
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