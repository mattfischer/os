#include "Object.hpp"

#include "Slab.hpp"
#include "Sched.hpp"
#include "Message.hpp"
#include "Task.hpp"
#include "Process.hpp"

#include <string.h>

//! Slab allocator for objects
Slab<Object> Object::sSlab;

/*!
 * \brief Constructor
 * \param parent Object parent, or 0
 * \param data Arbitrary data pointer
 */
Object::Object(Channel *channel, unsigned data)
 : mChannel(channel),
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

	if(active()) {
		// Construct a message object, and send it on this object's channel
		Message *message = new Message(Sched::current(), data(), *sendMsg, *replyMsg);
		ret = mChannel->send(message);
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

	if(active()) {
		// Construct an event message, and add it to the queue
		MessageEvent *event = new MessageEvent(Sched::current(), data(), type, value);
		mChannel->post(event);

		ret = SysErrorSuccess;
	} else {
		// If there are no server references to this object, the message can never
		// be received, so just return now
		ret = SysErrorObjectDead;
	}

	return ret;
}

void Object::onLastRef()
{
	post(SysEventObjectClosed, 0);
	delete this;
}

/*!
 * \brief Create an object
 * \param parent Object parent
 * \param data Data associated with object
 * \return New object id
 */
int Object_Create(int chan, unsigned data)
{
	Process *process = Sched::current()->process();
	Channel *channel = process->channel(chan);
	Object *object = new Object(channel, data);
	return process->refObject(object);
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
