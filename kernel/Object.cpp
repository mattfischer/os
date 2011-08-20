#include "Object.h"
#include "Slab.h"
#include "Sched.h"
#include "Util.h"

static struct SlabAllocator<struct Object> objectSlab;

struct Object *Object_Create()
{
	struct Object *object = objectSlab.allocate();
	LIST_INIT(object->receivers);
	LIST_INIT(object->messages);

	return object;
}

static int readBuffer(Process *destProcess, void *dest, Process *srcProcess, struct MessageHeader *src, int offset, int size, int translateCache[])
{
	int i, j;
	int copied;
	int srcOffset;

	copied = 0;
	srcOffset = 0;
	for(i=0; i<src->numSegments; i++) {
		struct BufferSegment segment;
		int segmentSize;
		int segmentStart;

		AddressSpace::memcpy(NULL, &segment, srcProcess->addressSpace(), src->segments + i, sizeof(struct BufferSegment));

		if(srcOffset + segment.size < offset) {
			srcOffset += segment.size;
			continue;
		}

		segmentStart = offset + copied - srcOffset;
		segmentSize = min(size - copied, segment.size - segmentStart);

		AddressSpace::memcpy(destProcess->addressSpace(), (char*)dest + copied, srcProcess->addressSpace(), (char*)segment.buffer + segmentStart, segmentSize);

		for(j=0; j<src->objectsSize; j++) {
			int *s, *d;
			int objOffset;
			int obj;

			objOffset = src->objectsOffset + j * sizeof(int);
			if(objOffset < srcOffset + segmentStart || objOffset >= srcOffset + segmentSize) {
				continue;
			}

			s = (int*)PADDR_TO_VADDR(srcProcess->addressSpace()->pageTable()->translateVAddr((char*)segment.buffer + objOffset - srcOffset));
			d = (int*)PADDR_TO_VADDR(destProcess->addressSpace()->pageTable()->translateVAddr((char*)dest + objOffset - offset));
			obj = *s;

			if(translateCache[i] == INVALID_OBJECT) {
				translateCache[i] = destProcess->dupObjectRef(srcProcess, obj);
			}

			*d = translateCache[i];
		}

		srcOffset += segment.size;
		copied += segmentSize;

		if(copied == size) {
			break;
		}
	}

	return copied;
}

static int copyBuffer(Process *destProcess, struct MessageHeader *dest, Process *srcProcess, struct MessageHeader *src, int translateCache[])
{
	int size;
	int copied;
	int i;

	dest->objectsOffset = src->objectsOffset;
	dest->objectsSize = src->objectsSize;

	copied = 0;
	for(i=0; i<dest->numSegments; i++) {
		struct BufferSegment segment;
		int segmentCopied;

		AddressSpace::memcpy(NULL, &segment, destProcess->addressSpace(), dest->segments + i, sizeof(struct BufferSegment));

		segmentCopied = readBuffer(destProcess, segment.buffer, srcProcess, src, copied, segment.size, translateCache);
		copied += segmentCopied;
		if(segmentCopied < segment.size) {
			break;
		}
	}

	return copied;
}

int Object_SendMessage(struct Object *object, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Message message;
	struct Task *task;
	int i;

	message.sender = Current;
	message.sendMsg = *sendMsg;
	message.replyMsg = *replyMsg;

	for(i=0; i<sendMsg->objectsSize; i++) {
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
	return readBuffer(Current->process, buffer, message->sender->process, &message->sendMsg, offset, size, message->translateCache);
}

int Object_ReplyMessage(struct Message *message, int ret, struct MessageHeader *replyMsg)
{
	int i;
	int translateCache[MESSAGE_MAX_OBJECTS];

	for(i=0; i<replyMsg->objectsSize; i++) {
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
	return Current->process->refObject(object);
}

void ReleaseObject(int obj)
{
	Current->process->unrefObject(obj);
}