#ifndef TASK_H
#define TASK_H

#include "AddressSpace.h"
#include "Page.h"
#include "List.h"
#include "Process.h"
#include "Slab.h"

#define R_SP 13
#define R_PC 15

class Process;

/*!
 * \brief A single thread of execution
 */
class Task : public ListEntry {
public:
	//! Task state
	enum State {
		StateInit, //!< Initialized, not yet started
		StateRunning, //!< Currently running on the CPU
		StateReady, //!< Ready to be run by the scheduler
		StateReceiveBlock, //!< Blocked on a message receive
		StateSendBlock, //!< Blocked on a message send
		StateReplyBlock //!< Blocked waiting for message reply
	};

	Task(Process *process);

	/*!
	 * \brief Owning process
	 * \return Process
	 */
	Process *process() { return mProcess; }
	/*!
	 * \brief Task state
	 * \return State
	 */
	State state() { return mState; }
	/*!
	 * \brief Set task state
	 * \param state New state
	 */
	void setState(State state) { mState = state; }
	/*!
	 * \brief Retrieve saved registers
	 * \return registers
	 */
	unsigned *regs() { return mRegs; }

	/*!
	 * \brief The address space currently being used by this task
	 *
	 * For processes that have an address space, this is the process's address
	 * space.  For kernel tasks, this is the address space of the last-executed
	 * non-kernel task.
	 * \return Effective address space
	 */
	AddressSpace *effectiveAddressSpace() { return mEffectiveAddressSpace; }
	/*!
	 * \brief Set the effective address space
	 * \param addressSpace New effective address space
	 */
	void setEffectiveAddressSpace(AddressSpace *addressSpace) { mEffectiveAddressSpace = addressSpace; }

	void *stackAllocate(int size);
	void start(void (*start)(void *), void *param);

	//! Allocator
	void *operator new(size_t size) { return sSlab.allocate(); }

private:
	unsigned int mRegs[16]; //!< Saved registers
	State mState; //!< Task state
	Page *mStack; //!< Stack page
	Process *mProcess; //!< Owning process
	AddressSpace *mEffectiveAddressSpace; //!< Effective address space

	static Slab<Task> sSlab;
};

#endif