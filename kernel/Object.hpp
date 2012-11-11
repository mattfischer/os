#ifndef OBJECT_H
#define OBJECT_H

#include "List.hpp"
#include "Slab.hpp"

#include <lib/shared/include/Object.h>

class Task;
class MessageBase;
class Message;
class Process;

/*!
 * \brief A kernel object, to which userspace can send/receive messages
 */
class Object : public ListEntry {
public:
	Object(Process *owner, Object *parent, void *data);

	int send(const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	void post(unsigned type, unsigned value);
	Message *receive(struct MessageHeader *recvMsg);

	void ref();
	void unref();

	void clientRef();
	void clientUnref();
	int clientRefCount() { return mClientRefCount; }

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
	/*!
	 * \brief Get owning process
	 * \return Owner
	 */
	Process *owner() { return mOwner; }

	//! Allocator
	void *operator new(size_t) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((Object*)p); }

private:
	List<Task> mReceivers; //!< List of receivers waiting on this object
	List<MessageBase> mMessages; //!< List of pending messages sent to this object
	List<Object> mSendingChildren; //!< List of children which have pending messages
	void *mData; //!< Arbitrary data associated with object
	Object *mParent; //!< Parent of this object
	Process *mOwner; //!< Owning process
	int mRefCount; //!< Ref count
	int mClientRefCount; //!< Client ref count

	Task *findReceiver();

	static Slab<Object> sSlab;
};

#endif
