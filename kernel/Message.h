#ifndef MESSAGE_H
#define MESSAGE_H

#include "List.h"
#include "Task.h"
#include "include/MessageFmt.h"

#include <lib/shared/include/Message.h>

/*!
 * \brief Base class for all messages
 */
class MessageBase : public ListEntry {
public:
	enum Type {
		TypeMessage, //!< Regular message
		TypeEvent //!< Event message
	};

	/*!
	 * \brief Constructor
	 * \param type Message type
	 * \param sender Sending task
	 * \param target Target object
	 */
	MessageBase(Type type, Task *sender, Object *target)
	 : mType(type),
	   mSender(sender),
	   mTarget(target)
	{}

	/*!
	 * \brief Return message type
	 * \return Type
	 */
	Type type() { return mType; }
	/*!
	 * \brief Return message sender
	 * \return Sender
	 */
	Task *sender() { return mSender; }
	/*!
	 * \brief Return message target
	 * \return Target
	 */
	Object *target() { return mTarget; }

	/*!
	 * \brief Abstract method.  Read message contents into header
	 * \param header Message header
	 * \return Number of bytes copied
	 */
	virtual int read(struct MessageHeader *header) = 0;

private:
	Type mType; //!< Message type
	Task *mSender; //!< Sending task
	Object *mTarget; //!< Target object
};

/*!
 * \brief Regular message
 */
class Message : public MessageBase {
public:
	Message(Task *sender, Object *target, struct MessageHeader &sendMsg, struct MessageHeader &replyMsg);

	/*!
	 * \brief Return send message area
	 * \return Header for sent message
	 */
	struct MessageHeader &sendMsg() { return mSendMsg; }
	/*!
	 * \brief Return reply message area
	 * \return Header for reply message
	 */
	struct MessageHeader &replyMsg() { return mReplyMsg; }

	/*!
	 * \brief Return code from message
	 * \return Return code
	 */
	int ret() { return mRet; }

	int read(void *buffer, int offset, int size);
	virtual int read(struct MessageHeader *header);

	int reply(int ret, struct MessageHeader *replyMsg);

	void info(struct MessageInfo *info);

private:
	struct MessageHeader mSendMsg; //!< Data area for sent message
	struct MessageHeader mReplyMsg; //!< Data area for message reply
	int mRet; //!< Return code
	int mTranslateCache[MESSAGE_MAX_OBJECTS]; //!< Cache of translated objects
};

/*!
 * \brief Event message
 */
class MessageEvent : public MessageBase {
public:
	MessageEvent(Task *sender, Object *target, unsigned type, unsigned value)
	: MessageBase(TypeEvent, sender, target),
	  mType(type),
	  mValue(value)
	{}

	virtual int read(struct MessageHeader *header);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((MessageEvent*)p); }

private:
	unsigned mType; //!< Event type
	unsigned mValue; //!< Event value

	static Slab<MessageEvent> sSlab;
};
#endif