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
	class Handle : public RefObject {
	public:
		enum Type {
			TypeServer,
			TypeClient
		};

		Type type() { return mType; }
		Object *object() { return mObject; }

		virtual void onFirstRef();
		virtual void onLastRef();

		friend class Object;
	protected:
		Handle(Object *object, Type type);

		Object *mObject;
		Type mType;
	};

	Object(Channel *channel, void *data);

	int send(const struct MessageHeader *sendMsg, struct MessageHeader *replyMsg);
	int post(unsigned type, unsigned value);

	/*!
	 * \brief Get data associated with object
	 * \return Data
	 */
	void *data() { return mData; }

	/*!
	 * \brief Get a handle of the specified type
	 * \param type Type of handle
	 * \return Handle
	 */
	Handle *handle(Handle::Type type);

	virtual void onLastRef();

	//! Allocator
	void *operator new(size_t) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((Object*)p); }

	friend class Handle;

private:
	Channel *mChannel;
	void *mData; //!< Arbitrary data associated with object
	Handle mClientHandle; //!< Client handle
	Handle mServerHandle; //!< Server handle

	void onHandleClosed(Handle::Type type);

	static Slab<Object> sSlab;
};

#endif
