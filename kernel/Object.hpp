#ifndef OBJECT_H
#define OBJECT_H

#include "List.hpp"
#include "Slab.hpp"
#include "Ref.hpp"
#include "Channel.hpp"

#include <lib/shared/include/Object.h>

#include <stddef.h>

class Task;
class MessageBase;
class Message;

/*!
 * \brief A kernel object, to which userspace can send/receive messages
 */
class Object : public ListEntry, public RefObject {
public:
	Object(Channel *channel, void *data);

	int send(const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	int post(unsigned type, unsigned value);

	/*!
	 * \brief Get data associated with object
	 * \return Data
	 */
	void *data() { return mData; }

	virtual void onLastRef();

	//! Allocator
	void *operator new(size_t) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((Object*)p); }

private:
	Ref<Channel> mChannel;
	void *mData; //!< Arbitrary data associated with object

	static Slab<Object> sSlab;
};

#endif
