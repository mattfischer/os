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

Task *Object::findReceiver()
{
	for(Object *object = this; object != NULL; object = object->parent()) {
		if(!object->mReceivers.empty()) {
			return object->mReceivers.removeHead();
		} else {
			Object *parent = object->parent();
			if(parent) {
				parent->mSendingChildren.remove(object);
				parent->mSendingChildren.addTail(object);
			}
		}
	}

	return NULL;
}

int Object::send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	Message message(Sched::current(), this, *sendMsg, *replyMsg);
	mMessages.addTail(&message);

	Task *task = findReceiver();

	Sched::current()->setState(Task::StateSendBlock);
	if(task) {
		Sched::switchTo(task);
	} else {
		Sched::runNext();
	}

	return message.ret();
}

void Object::post(unsigned type, unsigned value)
{
	MessageEvent *event = new MessageEvent(Sched::current(), this, type, value);
	mMessages.addTail(event);

	Task *task = findReceiver();

	if(task) {
		Sched::add(task);
	}
}

Message *Object::receive(struct MessageHeader *recvMsg)
{
	MessageBase *message = NULL;
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

	if(message->type() == Message::TypeMessage) {
		message->sender()->setState(Task::StateReplyBlock);
		return (Message*)message;
	} else {
		delete (MessageEvent*)message;
		return NULL;
	}
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

int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	return object->send(sendMsg, replyMsg);
}

void Object_Post(int obj, unsigned int type, unsigned int value)
{
	struct Object *object = Sched::current()->process()->object(obj);
	object->post(type, value);
}

int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	struct Message *message = object->receive(recvMsg);
	return Sched::current()->process()->refMessage(message);
}
