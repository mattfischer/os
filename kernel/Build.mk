TARGETS := kernel

kernel_SOURCES := \
	EntryAsm.s \
	Entry.c \
	Map.c \
	Page.c \
	Sched.c \
	Slab.c \
	SwitchTo.s \
	StartStub.c
kernel_LDFLAGS := -T $(CWD)ldscript
kernel_CFLAGS := -g
kernel_AFLAGS := -g
kernel_EXTRA_DEPS := $(CWD)ldscript
	
qemu: $(BINDIR)kernel
	qemu-system-arm -kernel $< -s -S

gdbcmd := /tmp/gdbcmd

gdb: $(BINDIR)kernel
	echo "target remote :1234" > $(gdbcmd)
	$(GDB) $< -x $(gdbcmd)
