#include "Interrupt.hpp"

#include "Page.hpp"
#include "List.hpp"
#include "Object.hpp"

#define N_INTERRUPTS 32

struct Subscription
{
	Object *object;
	unsigned type;
	unsigned value;
};

static Subscription subscriptions[N_INTERRUPTS];

#define PIC_BASE (unsigned*)PADDR_TO_VADDR((PAddr)0x14000000)
#define PIC_IRQ_STATUS    (PIC_BASE + 0)
#define PIC_IRQ_RAWSTAT   (PIC_BASE + 1)
#define PIC_IRQ_ENABLESET (PIC_BASE + 2)
#define PIC_IRQ_ENABLECLR (PIC_BASE + 3)

void Interrupt::init()
{
	for(int i=0; i<N_INTERRUPTS; i++) {
		subscribe(i, 0, 0, 0);
	}
}

bool Interrupt::subscribe(int irq, Object *object, unsigned type, unsigned value)
{
	if(subscriptions[irq].object) {
		if(subscriptions[irq].object->active()) {
			return false;
		} else {
			subscriptions[irq].object->unref();
		}
	}

	if(object) {
		object->ref();
	}
	subscriptions[irq].object = object;
	subscriptions[irq].type = type;
	subscriptions[irq].value = value;

	if(object && object->active()) {
		unmask(irq);
	}

	return true;
}

void Interrupt::mask(int irq)
{
	*PIC_IRQ_ENABLECLR = 1 << irq;
}

void Interrupt::unmask(int irq)
{
	*PIC_IRQ_ENABLESET = 1 << irq;
}

void Interrupt::dispatch()
{
	unsigned status = *PIC_IRQ_STATUS;

	for(int i=0; i<N_INTERRUPTS; i++) {
		if(status & 0x1) {
			if(subscriptions[i].object) {
				if(subscriptions[i].object->active()) {
					mask(i);
					subscriptions[i].object->post(subscriptions[i].type, subscriptions[i].value);
				} else {
					subscribe(i, 0, 0, 0);
				}
			}
		}
		status >>= 1;
	}
}
