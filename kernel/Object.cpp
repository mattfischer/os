#include "Object.h"
#include "Slab.h"
#include "Sched.h"
#include "Util.h"

Slab<Object> Object::sSlab;

Object::Object(Object *parent, void *data)
{
	mParent = parent;
	mData = data;
}

int Object::send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	Message message(Sched::current(), this, *sendMsg, *replyMsg);
	mMessages.addTail(&message);

	Sched::current()->setState(Task::StateSendBlock);

	Task *task = NULL;
	for(Object *object = this; object != NULL; object = object->parent()) {
		if(!object->mReceivers.empty()) {
			task = object->mReceivers.removeHead();
			break;
		} else {
			Object *parent = object->parent();
			if(parent) {
				parent->mSendingChildren.remove(object);
				parent->mSendingChildren.addTail(object);
			}
		}
	}

	if(task) {
		Sched::switchTo(task);
	} else {
		Sched::runNext();
	}

	return message.ret();
}

Message *Object::receive(struct MessageHeader *recvMsg)
{
	Message *message = NULL;
	while(!message) {
		for(Object *object = this; object != NULL; object = object->mSendingChildren.head()) {
			if(!object->mMessages.empty()) {
				message = object->mMessages.removeHead();

				for(; object != NULL; object = object->parent()) {
					if(object->mSendingChildren.empty() && object->mMessages.empty() && object->parent()) {
						object->parent()->mSendingChildren.remove(object);
					} else {
						break;
					}
				}
				break;
			}
		}

		if(message) {
			break;
		}

		mReceivers.addTail(Sched::current());
		Sched::current()->setState(Task::StateReceiveBlock);
		Sched::runNext();
	}

	message->read(recvMsg);
	message->sender()->setState(Task::StateReplyBlock);

	return message;
}

int Object_Create(int parent, void *data)
{
	Object *p = Sched::current()->process()->object(parent);
	Object *object = new Object(p, data);
	return Sched::current()->process()->refObject(object);
}

void Object_Release(int obj)
{
	Sched::current()->process()->unrefObject(obj);
}

int Object_Send(int obj, void *msg, int msgSize, void *reply, int replySize)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, &replyMsg);
}

int Object_Sendxs(int obj, struct MessageHeader *sendMsg, void *reply, int replySize)
{
	struct BufferSegment replySegs[] = { reply, replySize };
	struct MessageHeader replyMsg = { replySegs, 1, 0, 0 };

	return Object_Sendx(obj, sendMsg, &replyMsg);
}

int Object_Sendsx(int obj, void *msg, int msgSize, struct MessageHeader *replyMsg)
{
	struct BufferSegment sendSegs[] = { msg, msgSize };
	struct MessageHeader sendMsg = { sendSegs, 1, 0, 0 };

	return Object_Sendx(obj, &sendMsg, replyMsg);
}

int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	return object->send(sendMsg, replyMsg);
}

int Object_Receive(int obj, void *recv, int recvSize)
{
	struct BufferSegment recvSegs[] = { recv, recvSize };
	struct MessageHeader recvMsg = { recvSegs, 1, 0, 0 };

	return Object_Receivex(obj, &recvMsg);
}

int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	struct Message *message = object->receive(recvMsg);
	return Sched::current()->process()->refMessage(message);
}
