#ifndef CHANNEL_H
#define CHANNEL_H

#include "Slab.hpp"
#include "Message.hpp"
#include "Task.hpp"
#include "List.hpp"

#include "include/MessageFmt.h"
#include <lib/shared/include/Channel.h>

class Channel : public RefObject {
public:
	Channel();

	int send(Message *message);
	void post(MessageEvent *event);
	Message *receive(struct MessageHeader *recvMsg, unsigned *targetData);

	bool active() { return mActive; }
	void kill();

	void onLastRef();

	//! Allocator
	void *operator new(size_t) { return sSlab.allocate(); }
	void operator delete(void *p) { sSlab.free((Channel*)p); }

private:
	Task *findReceiver();

	List<Task> mReceivers; //!< List of receivers waiting on this object
	List<MessageBase> mMessages; //!< List of pending messages sent to this object
	bool mActive; //!< Channel active

	static Slab<Channel> sSlab;
};

#endif