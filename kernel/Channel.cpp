#include "Channel.hpp"

#include "Sched.hpp"
#include "Object.hpp"
#include "Process.hpp"

//! Slab allocator for objects
Slab<Channel> Channel::sSlab;

/*!
 * \brief Send a message to an object
 * \param sendMsg Message to send
 * \param replyMsg Message reply info
 * \return Message reply code
 */
void Channel::send(MessageBase *message)
{
	int ret;

	mMessages.addTail(message);

	// See if any tasks are ready to receive on this object or any of its ancestors
	Task *task = 0;
	while(!mReceivers.empty()) {
		// Found a receiver.  Remove it from the list and return it.
		task = mReceivers.removeHead();
		if(task->state() == Task::StateDead) {
			task->unref();
			continue;
		} else {
			break;
		}
	}

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
}

/*!
 * \brief Send a message to an object
 * \param sendMsg Message to send
 * \param replyMsg Message reply info
 * \return Message reply code
 */
void Channel::post(MessageBase *message)
{
	int ret;

	mMessages.addTail(message);

	// See if any tasks are ready to receive on this object or any of its ancestors
	Task *task = 0;
	while(!mReceivers.empty()) {
		// Found a receiver.  Remove it from the list and return it.
		task = mReceivers.removeHead();
		if(task->state() == Task::StateDead) {
			task->unref();
			continue;
		} else {
			break;
		}
	}

	if(task) {
		// Switch to the receiving task
		Sched::add(task);
		task->unref();
	}
}

/*!
 * \brief Receive a message from this object
 * \param recvMsg Receive message info
 * \return Message object
 */
Message *Channel::receive(struct MessageHeader *recvMsg)
{
	// Search down the object hierarchy, looking for a pending message
	// in this object or any of its children
	MessageBase *message = 0;
	while(!message) {
		if(!mMessages.empty()) {
			// Found an object with a non-empty message queue.  Remove the message.
			message = mMessages.removeHead();
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

/*!
 * \brief Create a channel
 * \return New channel id
 */
int Channel_Create()
{
	Process *process = Sched::current()->process();
	Channel *channel = new Channel();
	return process->refChannel(channel);
}

/*!
 * \brief Destroy a channel
 * \param chan Channel index
 */
void Channel_Destroy(int chan)
{
	Process *process = Sched::current()->process();
	process->unrefChannel(chan);
}

/*!
 * \brief Receive a message from an object
 * \param obj Object id
 * \param recvMsg Message receive area
 * \return Message id
 */
int Channel_Receivex(int chan, struct MessageHeader *recvMsg)
{
	Process *process = Sched::current()->process();
	Channel *channel = process->channel(chan);
	int ret;

	struct Message *message = channel->receive(recvMsg);
	ret = process->refMessage(message);

	return ret;
}
