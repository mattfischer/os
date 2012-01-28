#ifndef OBJECT_H
#define OBJECT_H

#include "List.h"
#include "Task.h"
#include "Message.h"

#include <lib/shared/include/Object.h>

/*!
 * \brief A kernel object, to which userspace can send/receive messages
 */
class Object : public ListEntry {
public:
	Object(Object *parent, void *data);

	int send(struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	void post(unsigned type, unsigned value);
	Message *receive(struct MessageHeader *recvMsg);

	/*!
	 * \brief Get parent of this object
	 * \return Object parent
	 */
	Object *parent() { return mParent; }
	/*!
	 * \brief Get data associated with object
	 * \return Data
	 */
	void *data() { return mData; }

	//! Allocator
	void *operator new(size_t) { return sSlab.allocate(); }

private:
	List<Task> mReceivers; //!< List of receivers waiting on this object
	List<MessageBase> mMessages; //!< List of pending messages sent to this object
	List<Object> mSendingChildren; //!< List of children which have pending messages
	void *mData; //!< Arbitrary data associated with object
	Object *mParent; //!< Parent of this object

	Task *findReceiver();

	static Slab<Object> sSlab;
};

#endif