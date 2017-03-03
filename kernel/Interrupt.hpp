#ifndef INTERRUPT_H
#define INTERRUPT_H

class Object;

class Interrupt
{
public:
	static void init();

	static void subscribe(int irq, Object *object, unsigned type, unsigned value);

	static void mask(int irq);
	static void unmask(int irq);

	static void dispatch();
};

#endif
