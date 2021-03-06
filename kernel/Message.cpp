#include "Message.hpp"

#include "Object.hpp"
#include "Sched.hpp"
#include "PageTable.hpp"
#include "Task.hpp"
#include "AddressSpace.hpp"
#include "Process.hpp"

#include <algorithm>

#include <string.h>

Slab<MessageEvent> MessageEvent::sSlab;
Slab<Message> Message::sSlab;

MessageBase::MessageBase(Type type, Task *sender, unsigned targetData)
 : mType(type),
   mSender(sender),
   mTargetData(targetData)
{
}

// Read data from a message into a buffer
static int readMessage(Process *destProcess, void *dest, Process *srcProcess, const struct MessageHeader *src, int offset, int size)
{
	// Iterate through as many source segments as it takes to copy the requested amount of data
	int copied = 0;
	int srcOffset = 0;
	for(int i=0; i<src->numSegments; i++) {
		// First, copy segment data out of the source message
		struct BufferSegment segment;
		AddressSpace::memcpy(0, &segment, srcProcess->addressSpace(), src->segments + i, sizeof(struct BufferSegment));

		// If this segment lies entirely before the offset of the source data, skip past it
		if(srcOffset + segment.size < offset) {
			srcOffset += segment.size;
			continue;
		}

		// Compute how far into the segment the copy should begin from, and how large it should be
		int segmentStart = offset + copied - srcOffset;
		int segmentSize = std::min(size - copied, segment.size - segmentStart);

		// Everything is now set up...copy the data
		AddressSpace::memcpy(destProcess->addressSpace(), (char*)dest + copied, srcProcess->addressSpace(), (char*)segment.buffer + segmentStart, segmentSize);

		// Now, translate any object references that were in this block
		for(int j=0; j<src->objectsSize; j++) {
			int objOffset = src->objectsOffset + j * sizeof(int);
			// If this object is located in a different segment, then skip it
			if(objOffset < srcOffset + segmentStart || objOffset >= srcOffset + segmentSize) {
				continue;
			}

			// Get the kernel addresses for the object slot in both the source and destination buffers
			int *s = (int*)PADDR_TO_VADDR(srcProcess->addressSpace()->pageTable()->translateVAddr((char*)segment.buffer + objOffset - srcOffset));
			int *d = (int*)PADDR_TO_VADDR(destProcess->addressSpace()->pageTable()->translateVAddr((char*)dest + objOffset - offset));

			// Get the object id in the source buffer
			int obj = *s;

			// Duplicate the object into the destination process
			*d = destProcess->dupObjectRef(srcProcess, obj);
		}

		// Record how much data was copied from this source segment
		srcOffset += segment.size;
		copied += segmentSize;

		if(copied == size) {
			// All data has been copied, so break
			break;
		}
	}

	return copied;
}

// Copy data from one process's message buffer to another
static int copyMessage(Process *destProcess, struct MessageHeader *dest, Process *srcProcess, const struct MessageHeader *src)
{
	dest->objectsOffset = src->objectsOffset;
	dest->objectsSize = src->objectsSize;

	// Iterate through the segments of the destination, copying enough data to fill each
	int copied = 0;
	for(int i=0; i<dest->numSegments; i++) {
		// First, get the segment information itself from the source message
		struct BufferSegment segment;
		AddressSpace::memcpy(0, &segment, destProcess->addressSpace(), dest->segments + i, sizeof(struct BufferSegment));

		// Now, fill this segment with data from the source message
		int segmentCopied = readMessage(destProcess, segment.buffer, srcProcess, src, copied, segment.size);

		// Record the amount of data copied
		copied += segmentCopied;
		if(segmentCopied < segment.size) {
			// We've reached the end of the source message
			break;
		}
	}

	return copied;
}

/*!
 * \brief Constructor
 * \param sender Sending task
 * \param targetData Target data
 * \param sendMsg Send message data
 * \param replyMsg Reply message data
 */
Message::Message(Task *sender, unsigned targetData, const struct MessageHeader &sendMsg, struct MessageHeader &replyMsg)
 : MessageBase(TypeMessage, sender, targetData)
{
	mSendMsg = sendMsg;
	mReplyMsg = replyMsg;
	mResult = 0;
}

/*!
 * \brief Read message data into a buffer
 * \param buffer Buffer to read into
 * \param offset Offset into message payload to read
 * \param size Size of data to read
 * \return Size of data read
 */
int Message::read(void *buffer, int offset, int size)
{
	return readMessage(Sched::current()->process(), buffer, sender()->process(), &mSendMsg, offset, size);
}

int Message::read(struct MessageHeader *header)
{
	return copyMessage(Sched::current()->process(), header, sender()->process(), &mSendMsg);
}

/*!
 * \brief Reply to message
 * \param ret Return code
 * \param replyMsg Reply data
 */
int Message::reply(int result, const struct MessageHeader *replyMsg)
{
	int ret;

	if(sender()->state() == Task::StateDead) {
		delete this;
		ret = SysErrorObjectDead;
	} else {
		// Copy contents into sending process's reply buffer
		copyMessage(sender()->process(), &mReplyMsg, Sched::current()->process(), replyMsg);
		mResult = result;

		// Switch back to the sending process, so that the corresponding send
		// call can return
		Sched::add(Sched::current());
		Sched::switchTo(sender());
		ret = SysErrorSuccess;
	}

	return ret;
}

void Message::cancel()
{
	MessageHeader replyMsg = { 0, 0, 0, 0 };
	reply(SysErrorObjectDead, &replyMsg);
}

int MessageEvent::read(struct MessageHeader *header)
{
	struct Event event;

	// Construct an Event to organize data
	event.type = mType;
	event.value = mValue;

	// Copy data into the header's segment
	memcpy(header->segments[0].buffer, &event, sizeof(event));

	return sizeof(event);
}

/*!
 * \brief Read data from a message
 * \param msg Message id
 * \param buffer Buffer to copy data into
 * \param offset Offset into message payload to copy
 * \param size Size of data to copy
 */
int Message_Read(int msg, void *buffer, int offset, int size)
{
	struct Message *message = Sched::current()->process()->message(msg);

	return message->read(buffer, offset, size);
}

/*!
 * \brief Reply to a message
 * \param msg Message id
 * \param ret Return code
 * \param replyMsg Reply data
 */
int Message_Replyx(int msg, int ret, const struct MessageHeader *replyMsg)
{
	if(msg == 0) {
		return 0;
	}

	struct Message *message = Sched::current()->process()->message(msg);
	int r = message->reply(ret, replyMsg);
	Sched::current()->process()->unrefMessage(msg);

	return r;
}
