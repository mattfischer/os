#include "Object.hpp"

#include "Slab.hpp"
#include "Sched.hpp"
#include "Message.hpp"
#include "Task.hpp"
#include "Process.hpp"

#include <string.h>

//! Slab allocator for objects
Slab<Object> Object::sSlab;

Object::Handle::Handle(Object *object, Type type)
{
	mType = type;
	mObject = object;
}

void Object::Handle::onFirstRef()
{
	mObject->ref();
}

void Object::Handle::onLastRef()
{
	mObject->onHandleClosed(mType);
	mObject->unref();
}

/*!
 * \brief Constructor
 * \param parent Object parent, or 0
 * \param data Arbitrary data pointer
 */
Object::Object(Channel *channel, void *data)
 : mChannel(channel),
   mClientHandle(this, Handle::TypeClient),
   mServerHandle(this, Handle::TypeServer),
   mData(data)
{
}

/*!
 * \brief Send a message to an object
 * \param sendMsg Message to send
 * \param replyMsg Message reply info
 * \return Message reply code
 */
int Object::send(const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	int ret;

	if(mServerHandle.refCount() > 0) {
		// Construct a message object, and add it to the list of pending objects
		Message *message = new Message(Sched::current(), this, *sendMsg, *replyMsg);

		mChannel->send(message);

		// Message receive and reply has been completed, and control has switched back
		// to this task.  Return the code from the message reply.
		ret = message->result();
		delete message;
	} else {
		// If there are no server references to this object, the message can never
		// be received, so just return with an error now
		ret = SysErrorObjectDead;
	}

	return ret;
}

/*!
 * \brief Post an event to the object
 * \param type Event type
 * \param value Event value
 */
int Object::post(unsigned type, unsigned value)
{
	int ret;

	if(mServerHandle.refCount() > 0) {
		// Construct an event message, and add it to the queue
		MessageEvent *event = new MessageEvent(Sched::current(), this, type, value);
		mChannel->post(event);

		ret = SysErrorSuccess;
	} else {
		// If there are no server references to this object, the message can never
		// be received, so just return now
		ret = SysErrorObjectDead;
	}

	return ret;
}

Object::Handle *Object::handle(Handle::Type type)
{
	switch(type) {
		case Handle::TypeClient:
			return &mClientHandle;

		case Handle::TypeServer:
			return &mServerHandle;
	}
}

void Object::onLastRef()
{
	delete this;
}

void Object::onHandleClosed(Handle::Type type)
{
	switch(type) {
		case Handle::TypeClient:
			post(SysEventObjectClosed, 0);
			break;

		case Handle::TypeServer:
			break;
	}
}

/*!
 * \brief Create an object
 * \param parent Object parent
 * \param data Data associated with object
 * \return New object id
 */
int Object_Create(int chan, void *data)
{
	Process *process = Sched::current()->process();
	Channel *channel = process->channel(chan);
	Object *object = new Object(channel, data);
	return process->refObject(object, Object::Handle::TypeServer);
}

/*!
 * \brief Release reference to this object
 * \param obj Object id
 */
void Object_Release(int obj)
{
	Sched::current()->process()->unrefObject(obj);
}

/*!
 * \brief Send a message to an object
 * \param obj Object id
 * \param sendMsg Message to send
 * \param replyMsg Reply message info
 * \return Reply return value
 */
int Object_Sendx(int obj, const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	Process *process = Sched::current()->process();
	Object *object = process->object(obj);

	return object->send(sendMsg, replyMsg);
}

/*!
 * \brief Post an event to an object's message queue
 * \param obj Object id
 * \param type Event type
 * \param value Event data
 */
int Object_Post(int obj, unsigned int type, unsigned int value)
{
	Process *process = Sched::current()->process();
	Object *object = process->object(obj);

	return object->post(type, value);
}
