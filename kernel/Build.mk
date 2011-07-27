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

MKINITFS := $(HOST_BINDIR)mkinitfs$(HOST_EXE_EXT)

initfs_in := $(CWD)InitFs.in
initfs_d := $(DEPDIR)kernel/InitFs.d
initfs_tmp := $(OBJDIR)kernel/InitFs.tmp
initfs_o := $(OBJDIR)kernel/InitFs.o

$(initfs_d): $(initfs_in) $(MKINITFS) $(makefile)
	@mkdir -p $(dir $@)
	@$(MKINITFS) -d $@ -o $(initfs_tmp) $(initfs_in)

$(initfs_tmp): $(initfs_in) $(MKINITFS) $(makefile)
	@echo "MKINITFS  $<"
	@mkdir -p $(dir $@)
	@$(MKINITFS) -o $@ $(initfs_in)

$(initfs_o): $(initfs_tmp) $(makefile)
	@mkdir -p $(dir $@)
	@$(OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.initfs $< $@

-include $(initfs_d)

kernel_EXTRA_OBJS := $(initfs_o)

qemu: $(BINDIR)kernel
	qemu-system-arm -kernel $< -s -S

gdbcmd := /tmp/gdbcmd

gdb: $(BINDIR)kernel
	echo "target remote :1234" > $(gdbcmd)
	$(GDB) $< -x $(gdbcmd)
