#include "Channel.hpp"

#include "Sched.hpp"
#include "Object.hpp"
#include "Process.hpp"

//! Slab allocator for objects
Slab<Channel> Channel::sSlab;

/*!
 * \brief Constructor
 */
Channel::Channel()
{
	mState = StateRunning;
}

/*!
 * \brief Find a task which can receive a pending message
 * \return Task, or 0
 */
Task *Channel::findReceiver()
{
	Task *task = 0;
	while(!mReceivers.empty()) {
		// Found a receiver.  Remove it from the list and return it.
		task = mReceivers.removeHead();
		if(task->state() == Task::StateDead) {
			task->unref();
			task = 0;
			continue;
		} else {
			break;
		}
	}

	return task;
}

/*!
 * \brief Send a message to a channel
 * \param message Message to send
 */
int Channel::send(Message *message)
{
	// Add the message to the message list
	mMessages.addTail(message);

	// Mark ourselves as send-blocked
	Sched::current()->setState(Task::StateSendBlock);

	// See if any tasks are ready to receive on this object or any of its ancestors
	Task *task = findReceiver();
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
	return message->result();
}

/*!
 * \brief Post an event to a channel
 * \param event Event to post
 */
void Channel::post(MessageEvent *event)
{
	// Add the message to the message list
	mMessages.addTail(event);

	// See if any tasks are ready to receive on this object or any of its ancestors
	Task *task = findReceiver();
	if(task) {
		// Switch to the receiving task
		Sched::add(task);
		task->unref();
	}
}

/*!
 * \brief Receive a message from this channel
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
 * \brief Kill a channel and free any pending messages
 */
void Channel::kill()
{
	MessageBase *next;
	for(MessageBase *message = mMessages.head(); message != 0; message = next) {
		next = mMessages.next(message);
		if(message->type() == Message::TypeMessage) {
			Message *m = (Message*)message;
			m->cancel();
		} else {
			delete message;
		}
	}

	mState = StateDead;
}

void Channel::onLastRef()
{
	delete this;
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
