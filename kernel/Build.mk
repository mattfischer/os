TARGETS := kernel

kernel_SOURCES := \
	AddressSpace.cpp \
	AsmFuncs.s \
	Elf.cpp \
	EntryAsm.s \
	Entry.cpp \
	InitFs.cpp \
	Kernel.cpp \
	Low.cpp \
	MemArea.cpp \
	Message.cpp \
	Object.cpp \
	Page.cpp \
	PageTable.cpp \
	ProcessManager.cpp \
	Process.cpp \
	Sched.cpp \
	Slab.cpp \
	Task.cpp \
	Util.cpp
kernel_LDFLAGS := -T $(CWD)ldscript
kernel_CFLAGS := -g
kernel_AFLAGS := -g
kernel_EXTRA_DEPS := $(CWD)ldscript
kernel_PLATFORM := TARGET_BARE
kernel_LIBS := shared

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
	@echo "Starting QEMU..."
	@qemu-system-arm -kernel $< -s -S

gdbcmd := /tmp/gdbcmd

gdb: $(CROSS_BINDIR)kernel
	@echo "target remote :1234" > $(gdbcmd)
	@echo "Starting GDB..."
	@$(CROSS_GDB) $< -x $(gdbcmd)
