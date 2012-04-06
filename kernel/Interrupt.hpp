#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "List.hpp"

class Object;

class Interrupt
{
public:
	class Subscription : public ListEntry
	{
	public:
		Subscription() { mAcknowledged = false; }

		virtual void dispatch() = 0;
		bool acknowledged() { return mAcknowledged; }
		void setAcknowledged(bool acknowledged) { mAcknowledged = acknowledged; }

	private:
		bool mAcknowledged;
	};

	static void subscribe(unsigned irq, Subscription *subscription);
	static void unsubscribe(unsigned irq, Subscription *subscription);
	static void acknowledge(unsigned irq, Subscription *subscription);

	static void dispatch();
};

#endif
