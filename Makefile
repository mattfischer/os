SOURCES := \
	Entry.s \
	EntryHigh.c \
	Memory.c \
	Sched.c \
	SwitchTo.s \
	StartStub.c

CROSS_COMPILE := arm-none-eabi
GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb

CFLAGS := -g
AFLAGS := -g

C_SOURCES := $(filter %.c,$(SOURCES))
S_SOURCES := $(filter %.s,$(SOURCES))

OUTDIR := out
OBJDIR := $(OUTDIR)/obj
BINDIR := $(OUTDIR)/bin
DEPDIR := $(OUTDIR)/deps

OBJS := $(C_SOURCES:%.c=$(OBJDIR)/%.o) $(S_SOURCES:%.s=$(OBJDIR)/%.o)
DEPS := $(C_SOURCES:%.c=$(DEPDIR)/%.d)

KERNEL := $(BINDIR)/kernel

all: $(KERNEL)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
qemu: $(KERNEL)
	qemu-system-arm -kernel $< -s -S

gdb: $(KERNEL)
	echo "target remote :1234" > .gdbinit
	$(GDB) $< 

$(KERNEL): $(OBJS)
	@mkdir -p $(dir $@)
	@$(LD) $(OBJS) -o $@ -T ldscript
	@echo "LD    $@"

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@mkdir -p $(DEPDIR)
	@$(GCC) $(CFLAGS) -MP -MD -MF $(<:%.c=$(DEPDIR)/%.d) -c -o $@ $<
	@echo "CC    $<"
	
$(OBJDIR)/%.o: %.s
	@mkdir -p $(dir $@)
	@$(AS) $(AFLAGS) -o $@ $<
	@echo "AS    $<"
	
-include $(DEPS)
