#include "Object.h"
#include "Slab.h"
#include "Sched.h"
#include "Util.h"

Slab<Object> Object::sSlab;

Object::Object()
{
}

int Object::send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	Message message(Sched::current(), *sendMsg, *replyMsg);
	mMessages.addTail(&message);

	Sched::current()->setState(Task::StateSendBlock);
	if(!mReceivers.empty()) {
		Task *task = mReceivers.removeHead();
		Sched::switchTo(task);
	} else {
		Sched::runNext();
	}

	return message.ret();
}

Message *Object::receive(struct MessageHeader *recvMsg)
{
	if(mMessages.empty()) {
		mReceivers.addTail(Sched::current());
		Sched::current()->setState(Task::StateReceiveBlock);
		Sched::runNext();
	}

	Message *message = mMessages.removeHead();

	message->read(recvMsg);

	message->sender()->setState(Task::StateReplyBlock);

	return message;
}

int CreateObject()
{
	Object *object = new Object();
	return Sched::current()->process()->refObject(object);
}

void ReleaseObject(int obj)
{
	Sched::current()->process()->unrefObject(obj);
}