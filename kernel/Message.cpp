#include "Message.h"

#include "Object.h"
#include "Sched.h"
#include "Util.h"

static int readMessage(Process *destProcess, void *dest, Process *srcProcess, struct MessageHeader *src, int offset, int size, int translateCache[])
{
	int copied = 0;
	int srcOffset = 0;
	for(int i=0; i<src->numSegments; i++) {
		struct BufferSegment segment;

		AddressSpace::memcpy(NULL, &segment, srcProcess->addressSpace(), src->segments + i, sizeof(struct BufferSegment));

		if(srcOffset + segment.size < offset) {
			srcOffset += segment.size;
			continue;
		}

		int segmentStart = offset + copied - srcOffset;
		int segmentSize = min(size - copied, segment.size - segmentStart);

		AddressSpace::memcpy(destProcess->addressSpace(), (char*)dest + copied, srcProcess->addressSpace(), (char*)segment.buffer + segmentStart, segmentSize);

		for(int j=0; j<src->objectsSize; j++) {
			int objOffset = src->objectsOffset + j * sizeof(int);
			if(objOffset < srcOffset + segmentStart || objOffset >= srcOffset + segmentSize) {
				continue;
			}

			int *s = (int*)PADDR_TO_VADDR(srcProcess->addressSpace()->pageTable()->translateVAddr((char*)segment.buffer + objOffset - srcOffset));
			int *d = (int*)PADDR_TO_VADDR(destProcess->addressSpace()->pageTable()->translateVAddr((char*)dest + objOffset - offset));
			int obj = *s;

			if(translateCache[i] == OBJECT_INVALID) {
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

static int copyMessage(Process *destProcess, struct MessageHeader *dest, Process *srcProcess, struct MessageHeader *src, int translateCache[])
{
	dest->objectsOffset = src->objectsOffset;
	dest->objectsSize = src->objectsSize;

	int copied = 0;
	for(int i=0; i<dest->numSegments; i++) {
		struct BufferSegment segment;
		AddressSpace::memcpy(NULL, &segment, destProcess->addressSpace(), dest->segments + i, sizeof(struct BufferSegment));

		int segmentCopied = readMessage(destProcess, segment.buffer, srcProcess, src, copied, segment.size, translateCache);
		copied += segmentCopied;
		if(segmentCopied < segment.size) {
			break;
		}
	}

	return copied;
}

Message::Message(Task *sender, Object *target, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg)
{
	mSender = sender;
	mTarget = target;
	mSendMsg = sendMsg;
	mReplyMsg = replyMsg;
	mRet = 0;

	for(int i=0; i<mSendMsg.objectsSize; i++) {
		mTranslateCache[i] = OBJECT_INVALID;
	}
}

int Message::read(void *buffer, int offset, int size)
{
	return readMessage(Sched::current()->process(), buffer, mSender->process(), &mSendMsg, offset, size, mTranslateCache);
}

int Message::read(struct MessageHeader *header)
{
	return copyMessage(Sched::current()->process(), header, mSender->process(), &mSendMsg, mTranslateCache);
}

int Message::reply(int ret, struct MessageHeader *replyMsg)
{
	int translateCache[MESSAGE_MAX_OBJECTS];
	for(int i=0; i<replyMsg->objectsSize; i++) {
		translateCache[i] = OBJECT_INVALID;
	}

	copyMessage(mSender->process(), &mReplyMsg, Sched::current()->process(), replyMsg, translateCache);
	mRet = ret;

	Sched::add(Sched::current());
	Sched::switchTo(mSender);

	return 0;
}

void Message::info(struct MessageInfo *info)
{
	info->targetData = mTarget->data();
}

int Message_Read(int msg, void *buffer, int offset, int size)
{
	struct Message *message = Sched::current()->process()->message(msg);

	return message->read(buffer, offset, size);
}

int Message_Replyx(int msg, int ret, struct MessageHeader *replyMsg)
{
	struct Message *message = Sched::current()->process()->message(msg);
	int r = message->reply(ret, replyMsg);
	Sched::current()->process()->unrefMessage(msg);

	return r;
}

void Message_Info(int msg, struct MessageInfo *info)
{
	struct Message *message = Sched::current()->process()->message(msg);
	message->info(info);
}