#ifndef TASK_H
#define TASK_H

#include "AddressSpace.h"
#include "Defs.h"
#include "Page.h"
#include "List.h"
#include "Process.h"
#include "Slab.h"

#define R_SP 13
#define R_PC 15

class Process;

class Task : public ListEntry {
public:
	enum State {
		StateInit,
		StateRunning,
		StateReady,
		StateReceiveBlock,
		StateSendBlock,
		StateReplyBlock
	};

	Task(Process *process);

	Process *process() { return mProcess; }
	State state() { return mState; }
	void setState(State state) { mState = state; }

	AddressSpace *effectiveAddressSpace() { return mEffectiveAddressSpace; }
	void setEffectiveAddressSpace(AddressSpace *addressSpace) { mEffectiveAddressSpace = addressSpace; }

	void *stackAllocate(int size);
	void start(void (*start)(void *), void *param);

	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	unsigned int mRegs[16];
	State mState;
	struct Page *mStack;
	Process *mProcess;
	AddressSpace *mEffectiveAddressSpace;

	static SlabAllocator<Task> sSlab;
};

#endif