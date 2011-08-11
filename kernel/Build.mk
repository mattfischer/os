TARGETS := kernel

kernel_SOURCES := \
	AddressSpace.c \
	AsmFuncs.s \
	Elf.c \
	EntryAsm.s \
	Entry.c \
	InitFs.c \
	MemArea.c \
	Name.c \
	Object.c \
	Page.c \
	PageTable.c \
	ProcManager.c \
	Process.c \
	Sched.c \
	Slab.c \
	Task.c \
	Util.c
kernel_LDFLAGS := -T $(CWD)ldscript
kernel_CFLAGS := -g
kernel_AFLAGS := -g
kernel_EXTRA_DEPS := $(CWD)ldscript

MKINITFS := $(HOST_BINDIR)mkinitfs$(HOST_EXE_EXT)

initfs_in := $(CWD)InitFs.ifs
initfs_d := $(CROSS_DEPDIR)kernel/InitFsData.d
initfs_o := $(CROSS_OBJDIR)kernel/InitFsData.o

$(initfs_d): $(initfs_in) $(MKINITFS) $(makefile)
	@mkdir -p $(dir $@)
	@$(MKINITFS) -d $@ -o $(initfs_o) $(initfs_in)

$(initfs_o): $(initfs_in) $(MKINITFS) $(makefile)
	@echo "  MKINITFS  $<"
	@mkdir -p $(dir $@)
	@$(MKINITFS) -o $@.tmp $(initfs_in)
	@$(CROSS_OBJCOPY) -I binary -O elf32-littlearm -B arm --rename-section .data=.initfs $@.tmp $@
	@rm $@.tmp

-include $(initfs_d)

kernel_EXTRA_OBJS := $(initfs_o)

qemu: $(CROSS_BINDIR)kernel
	qemu-system-arm -kernel $< -s -S

gdbcmd := /tmp/gdbcmd

gdb: $(CROSS_BINDIR)kernel
	echo "target remote :1234" > $(gdbcmd)
	$(CROSS_GDB) $< -x $(gdbcmd)
