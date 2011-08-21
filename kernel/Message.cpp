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

Message::Message(Task *sender, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg)
{
	mSender = sender;
	mSendMsg = sendMsg;
	mReplyMsg = replyMsg;
	mRet = 0;

	for(int i=0; i<mSendMsg.objectsSize; i++) {
		mTranslateCache[i] = INVALID_OBJECT;
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
		translateCache[i] = INVALID_OBJECT;
	}

	copyMessage(mSender->process(), &mReplyMsg, Sched::current()->process(), replyMsg, translateCache);
	mRet = ret;

	Sched::add(Sched::current());
	Sched::switchTo(mSender);

	return 0;
}

int SendMessage(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return SendMessagex(obj, &sendMsg, &replyMsg);
}

int SendMessagexs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return SendMessagex(obj, sendMsg, &replyMsg);
}

int SendMessagesx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return SendMessagex(obj, &sendMsg, replyMsg);
}

int SendMessagex(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	return object->send(sendMsg, replyMsg);
}

int ReceiveMessage(int obj, void *recv, int recvSize)
{
	struct BufferSegment recvSegs[] = { recv, recvSize };
	struct MessageHeader recvMsg = { recvSegs, 1, 0, 0 };

	return ReceiveMessagex(obj, &recvMsg);
}

int ReceiveMessagex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	struct Message *message = object->receive(recvMsg);
	return Sched::current()->process()->refMessage(message);
}

int ReadMessage(int msg, void *buffer, int offset, int size)
{
	struct Message *message = Sched::current()->process()->message(msg);

	return message->read(buffer, offset, size);
}

int ReplyMessage(int msg, int ret, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return ReplyMessagex(msg, ret, &replyMsg);
}

int ReplyMessagex(int msg, int ret, struct MessageHeader *replyMsg)
{
	struct Message *message = Sched::current()->process()->message(msg);
	int r = message->reply(ret, replyMsg);
	Sched::current()->process()->unrefMessage(msg);

	return r;
}
