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
		Subscription(int irq);

		int irq() { return mIrq; }

		virtual void dispatch() = 0;
		bool acknowledged() { return mAcknowledged; }
		void setAcknowledged(bool acknowledged) { mAcknowledged = acknowledged; }

	private:
		bool mAcknowledged;
		int mIrq;
	};

	static void subscribe(Subscription *subscription);
	static void unsubscribe(Subscription *subscription);
	static void acknowledge(Subscription *subscription);

	static void dispatch();
};

#endif
