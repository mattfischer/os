OBJS := \
	entry.o \
	sched.o \
	switch_to.o \
	start_stub.o
	
CROSS_COMPILE := arm-none-eabi
GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb

CFLAGS := -g
AFLAGS := -g

all: kernel

clean:
	rm $(OBJS)
	rm kernel

qemu: kernel
	qemu-system-arm -kernel kernel -s -S

gdb: kernel
	$(GDB) $< 

kernel: $(OBJS)
	$(LD) $(OBJS) -o $@ -T ldscript

.c.o: 
	$(GCC) $(CFLAGS) -c -o $@ $<
	
.s.o:
	$(AS) $(AFLAGS) -o $@ $<