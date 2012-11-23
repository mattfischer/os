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
Object::Object(Object *parent, void *data)
 : mClientHandle(this, Handle::TypeClient),
   mServerHandle(this, Handle::TypeServer)
{
	mParent = parent;
	mData = data;
}

// Find an object in the hierarchy which is ready to receive a message
Task *Object::findReceiver()
{
	for(Object *object = this; object != 0; object = object->parent()) {
		while(!object->mReceivers.empty()) {
			// Found a receiver.  Remove it from the list and return it.
			Task *receiver = object->mReceivers.removeHead();
			if(receiver->state() == Task::StateDead) {
				receiver->unref();
				continue;
			} else {
				return receiver;
			}
		}

		Object *parent = object->parent();
		if(parent) {
			// Add this object to the end of the parent's send list
			parent->mSendingChildren.remove(object);
			parent->mSendingChildren.addTail(object);
			object->ref();
		}
	}

	// No receivers found
	return 0;
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
		mMessages.addTail(message);

		// See if any tasks are ready to receive on this object or any of its ancestors
		Task *task = findReceiver();

		// Mark ourselves as send-blocked
		Sched::current()->setState(Task::StateSendBlock);

		if(task) {
			// Switch to the receiving task
			Sched::switchTo(task);
			task->unref();
		} else {
			// Switch away from this task.  We will be woken up when another task attempts
			// to receive on this object.
			Sched::runNext();
		}

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
		mMessages.addTail(event);

		// See if any receivers are waiting on this object
		Task *task = findReceiver();

		// If a receiver was found, wake it up
		if(task) {
			Sched::add(task);
			task->unref();
		}

		ret =SysErrorSuccess;
	} else {
		// If there are no server references to this object, the message can never
		// be received, so just return now
		ret = SysErrorObjectDead;
	}

	return ret;
}

/*!
 * \brief Receive a message from this object
 * \param recvMsg Receive message info
 * \return Message object
 */
Message *Object::receive(struct MessageHeader *recvMsg)
{
	// Search down the object hierarchy, looking for a pending message
	// in this object or any of its children
	MessageBase *message = 0;
	while(!message) {
		for(Object *object = this; object != 0; object = object->mSendingChildren.head()) {
			if(!object->mMessages.empty()) {
				// Found an object with a non-empty message queue.  Remove the message.
				message = object->mMessages.removeHead();

				// Now that the message has been removed from the object's queue, it needs to
				// be removed from its parent's sendingChildren list.  This may in turn cause
				// other ancestors to be removed from their parents' lists as well.  Move up the
				// rest of the tree and check if any sending children lists need to be updated
				for(; object != 0; object = object->parent()) {
					if(object->mSendingChildren.empty() && object->mMessages.empty() && object->parent()) {
						// This object has no sending children itself, and no messages in its queue.
						// Therefore, it no longer belongs in its parent's sending children list.
						object->parent()->mSendingChildren.remove(object);
						object->unref();
					} else {
						// No changes need to be made to this object's parent, so no changes can
						// occur to any further parent.  Therefore, it's ok to stop searching now.
						break;
					}
				}
				break;
			}
		}

		// If we found a message, then break and process it
		if(message) {
			if(message->sender()->state() == Task::StateDead) {
				delete message;
				message = 0;
				continue;
			} else {
				break;
			}
		}

		// Otherwise, block until a message comes in.  Add ourselves to the object's
		// receiver list, and switch away from this task.
		mReceivers.addTail(Sched::current());
		Sched::current()->ref();
		Sched::current()->setState(Task::StateReceiveBlock);
		Sched::runNext();
	}

	// Read the message contents into this task's address space
	message->read(recvMsg);

	if(message->type() == Message::TypeMessage) {
		// Received message was a normal message.  Mark the sender as reply-blocked,
		// and return the message to the caller
		message->sender()->setState(Task::StateReplyBlock);
		return (Message*)message;
	} else {
		// Received message was an event.  No reply is required, so delete the message
		// from the queue and return.
		delete (MessageEvent*)message;
		return 0;
	}
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
			MessageBase *next;
			for(MessageBase *message = mMessages.head(); message != 0; message = next) {
				next = mMessages.next(message);
				if(message->type() == Message::TypeMessage) {
					Message *m = (Message*)message;
					m->cancel();
				}
			}

			if(mParent && mSendingChildren.empty()) {
				mParent->mSendingChildren.remove(this);
			}
			break;
	}
}

/*!
 * \brief Create an object
 * \param parent Object parent
 * \param data Data associated with object
 * \return New object id
 */
int Object_Create(int parent, void *data)
{
	Process *process = Sched::current()->process();
	Object *p = process->object(parent);
	Object *object = new Object(p, data);
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

/*!
 * \brief Receive a message from an object
 * \param obj Object id
 * \param recvMsg Message receive area
 * \return Message id
 */
int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	Process *process = Sched::current()->process();
	Object::Handle *handle = process->objectHandle(obj);
	int ret;

	if(handle->type() == Object::Handle::TypeServer) {
		struct Message *message = handle->object()->receive(recvMsg);
		ret = process->refMessage(message);
	} else {
		ret = SysErrorAccessDenied;
	}

	return ret;
}
