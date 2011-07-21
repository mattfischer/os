OBJS := \
	entry.o \
	sched.o \
	switch_to.o \
	start_stub.o
	
CROSS_COMPILE := arm-none-eabi
GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
OBJCOPY := $(CROSS_COMPILE)-objcopy
GDB := $(CROSS_COMPILE)-gdb

CFLAGS := -g
AFLAGS := -g

all: kernel.bin

clean:
	rm $(OBJS)
	rm kernel.elf
	rm kernel.bin

qemu: kernel.bin
	qemu-system-arm -kernel kernel.bin -s

gdb: kernel.elf
	$(GDB) $< 

kernel.elf: $(OBJS)
	$(LD) $(OBJS) -o $@ -T ldscript

.c.o: 
	$(GCC) $(CFLAGS) -c -o $@ $<
	
.s.o:
	$(AS) $(AFLAGS) -o $@ $<
	
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@
