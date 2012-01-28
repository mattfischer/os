#include "Object.h"
#include "Slab.h"
#include "Sched.h"
#include "Util.h"

//! Slab allocator for objects
Slab<Object> Object::sSlab;

/*!
 * \brief Constructor
 * \param parent Object parent, or NULL
 * \param data Arbitrary data pointer
 */
Object::Object(Object *parent, void *data)
{
	mParent = parent;
	mData = data;
}

// Find an object in the hierarchy which is ready to receive a message
Task *Object::findReceiver()
{
	for(Object *object = this; object != NULL; object = object->parent()) {
		if(!object->mReceivers.empty()) {
			// Found a receiver.  Remove it from the list and return it.
			return object->mReceivers.removeHead();
		} else {
			Object *parent = object->parent();
			if(parent) {
				// Add this object to the end of the parent's send list
				parent->mSendingChildren.remove(object);
				parent->mSendingChildren.addTail(object);
			}
		}
	}

	return NULL;
}

/*!
 * \brief Send a message to an object
 * \param sendMsg Message to send
 * \param replyMsg Message reply info
 * \return Message reply code
 */
int Object::send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	// Construct a message object, and add it to the list of pending objects
	Message message(Sched::current(), this, *sendMsg, *replyMsg);
	mMessages.addTail(&message);

	// See if any tasks are ready to receive on this object or any of its ancestors
	Task *task = findReceiver();

	// Mark ourselves as send-blocked
	Sched::current()->setState(Task::StateSendBlock);

	if(task) {
		// Switch to the receiving task
		Sched::switchTo(task);
	} else {
		// Switch away from this task.  We will be woken up when another task attempts
		// to receive on this object.
		Sched::runNext();
	}

	// Message receive and reply has been completed, and control has switched back
	// to this task.  Return the code from the message reply.
	return message.ret();
}

/*!
 * \brief Post an event to the object
 * \param type Event type
 * \param value Event value
 */
void Object::post(unsigned type, unsigned value)
{
	// Construct an event message, and add it to the queue
	MessageEvent *event = new MessageEvent(Sched::current(), this, type, value);
	mMessages.addTail(event);

	// See if any receivers are waiting on this object
	Task *task = findReceiver();

	// If a receiver was found, wake it up
	if(task) {
		Sched::add(task);
	}
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
	MessageBase *message = NULL;
	while(!message) {
		for(Object *object = this; object != NULL; object = object->mSendingChildren.head()) {
			if(!object->mMessages.empty()) {
				// Found an object with a non-empty message queue.  Remove the message.
				message = object->mMessages.removeHead();

				// Now that the message has been removed from the object's queue, higher
				// objects in the tree may no longer have any sending children.  Move up
				// the rest of the tree and check if any sending children lists need to
				// be updated
				for(; object != NULL; object = object->parent()) {
					if(object->mSendingChildren.empty() && object->mMessages.empty() && object->parent()) {
						// This object has no sending children itself, and no messages in its queue.
						// Therefore, it no longer belongs in its parent's sending children list.
						object->parent()->mSendingChildren.remove(object);
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
			break;
		}

		// Otherwise, block until a message comes in.  Add ourselves to the object's
		// receiver list, and switch away from this task.
		mReceivers.addTail(Sched::current());
		Sched::current()->setState(Task::StateReceiveBlock);
		Sched::runNext();
	}

	// Read the message contents into this task's address space
	message->read(recvMsg);

	if(message->type() == Message::TypeMessage) {
		// Received message was a normal message.  Mark the task as reply-blocked,
		// and return the message to the caller
		message->sender()->setState(Task::StateReplyBlock);
		return (Message*)message;
	} else {
		// Received message was an event.  No reply is required, so delete the message
		// from the queue and return.
		delete (MessageEvent*)message;
		return NULL;
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
	Object *p = Sched::current()->process()->object(parent);
	Object *object = new Object(p, data);
	return Sched::current()->process()->refObject(object);
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
int Object_Sendx(int obj, struct MessageHeader *sendMsg, struct MessageHeader *replyMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	return object->send(sendMsg, replyMsg);
}

/*!
 * \brief Post an event to an object's message queue
 * \param obj Object id
 * \param type Event type
 * \param value Event data
 */
void Object_Post(int obj, unsigned int type, unsigned int value)
{
	struct Object *object = Sched::current()->process()->object(obj);
	object->post(type, value);
}

/*!
 * \brief Receive a message from an object
 * \param obj Object id
 * \param recvMsg Message receive area
 * \return Message id
 */
int Object_Receivex(int obj, struct MessageHeader *recvMsg)
{
	struct Object *object = Sched::current()->process()->object(obj);
	struct Message *message = object->receive(recvMsg);
	return Sched::current()->process()->refMessage(message);
}
