TARGETS := kernel

kernel_SOURCES := \
	Entry.s \
	EntryHigh.c \
	Memory.c \
	Sched.c \
	SwitchTo.s \
	StartStub.c
kernel_LDFLAGS := -T $(CWD)ldscript
kernel_CFLAGS := -g
kernel_AFLAGS := -g
	
qemu: $(BINDIR)kernel
	qemu-system-arm -kernel $< -s -S

gdbcmd := /tmp/gdbcmd

gdb: $(BINDIR)kernel
	echo "target remote :1234" > $(gdbcmd)
	$(GDB) $< -x $(gdbcmd)
