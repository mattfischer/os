#include "Interrupt.hpp"

#include "Page.hpp"
#include "List.hpp"

#define N_INTERRUPTS 32

static List<Interrupt::Subscription> subscriptions[N_INTERRUPTS];
static int outstanding[N_INTERRUPTS];

#define PIC_BASE (unsigned*)PADDR_TO_VADDR((PAddr)0x14000000)
#define PIC_IRQ_STATUS    (PIC_BASE + 0)
#define PIC_IRQ_RAWSTAT   (PIC_BASE + 1)
#define PIC_IRQ_ENABLESET (PIC_BASE + 2)
#define PIC_IRQ_ENABLECLR (PIC_BASE + 3)

static void mask(unsigned irq)
{
	*PIC_IRQ_ENABLECLR = 1 << irq;
}

static void unmask(unsigned irq)
{
	*PIC_IRQ_ENABLESET = 1 << irq;
}

Interrupt::Subscription::Subscription(int irq)
{
	mAcknowledged = false;
	mIrq = irq;
}

void Interrupt::subscribe(Subscription *subscription)
{
	int irq = subscription->irq();

	subscriptions[irq].addTail(subscription);
	unmask(irq);
}

void Interrupt::unsubscribe(Subscription *subscription)
{
	int irq = subscription->irq();

	subscriptions[irq].remove(subscription);
	if(subscriptions[irq].empty()) {
		mask(irq);
	}
}

void Interrupt::acknowledge(Subscription *subscription)
{
	int irq = subscription->irq();

	if(!subscription->acknowledged()) {
		subscription->setAcknowledged(true);
		outstanding[irq]--;
		if(outstanding[irq] == 0) {
			unmask(irq);
		}
	}
}

void Interrupt::dispatch()
{
	unsigned status = *PIC_IRQ_STATUS;

	for(int i=0; i<N_INTERRUPTS; i++) {
		if(status & 0x1) {
			outstanding[i] = 0;
			Subscription *subscription;
			for(subscription = subscriptions[i].head(); subscription != NULL; subscription = subscriptions[i].next(subscription)) {
				subscription->setAcknowledged(false);
				subscription->dispatch();
				outstanding[i]++;
			}
			mask(i);
		}
		status >>= 1;
	}
}
