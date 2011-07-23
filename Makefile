TARGETS := kernel

kernel_SOURCES := \
	Entry.s \
	EntryHigh.c \
	Memory.c \
	Sched.c \
	SwitchTo.s \
	StartStub.c
kernel_LDFLAGS := -T ldscript
kernel_CFLAGS := -g
kernel_AFLAGS := -g

CROSS_COMPILE := arm-none-eabi
GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb

CFLAGS := -g
AFLAGS := -g

OUTDIR := out
OBJDIR := $(OUTDIR)/obj
BINDIR := $(OUTDIR)/bin
DEPDIR := $(OUTDIR)/deps

define build
$(1)_c_sources := $$(filter %.c,$$($(1)_SOURCES))
$(1)_s_sources := $$(filter %.s,$$($(1)_SOURCES))
$(1)_objdir := $(OBJDIR)/$(1)
$(1)_depdir := $(DEPDIR)/$(1)
$(1)_objs := $$($(1)_c_sources:%.c=$$($(1)_objdir)/%.o) $$($(1)_s_sources:%.s=$$($(1)_objdir)/%.o)

$(BINDIR)/$(1): $$($(1)_objs)
	@mkdir -p $$(dir $$@)
	@$(LD) $$($(1)_objs) -o $$@ $$($(1)_LDFLAGS)
	@echo "LD    $$@"

$$($(1)_objdir)/%.o: %.c
	@mkdir -p $$(dir $$@)
	@mkdir -p $$($(1)_depdir)
	@$(GCC) $$($(1)_CFLAGS) -MP -MD -MF $$(<:%.c=$$($(1)_depdir)/%.d) -c -o $$@ $$<
	@echo "CC    $$<"
	
$$($(1)_objdir)/%.o: %.s
	@mkdir -p $$(dir $$@)
	@$(AS) $(AFLAGS) -o $$@ $$<
	@echo "AS    $$<"

ALL_TARGETS += $(BINDIR)/$(1)
ALL_DEPS += $$($(1)_c_sources:%.c=$$($(1)_depdir)/%.d)
endef

$(foreach target,$(TARGETS),$(eval $(call build,$(target))))
all: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
qemu: $(BINDIR)/kernel
	qemu-system-arm -kernel $< -s -S

gdb: $(BINDIR)/kernel
	echo "target remote :1234" > .gdbinit
	$(GDB) $< 

-include $(ALL_DEPS)
