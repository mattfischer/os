CROSS_COMPILE := arm-none-eabi
GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb

OUTDIR := out/
OBJDIR := $(OUTDIR)obj/
BINDIR := $(OUTDIR)bin/
DEPDIR := $(OUTDIR)deps/

define build
$(1)_c_sources := $$(filter %.c,$$($(1)_SOURCES))
$(1)_s_sources := $$(filter %.s,$$($(1)_SOURCES))
$(1)_objdir := $(OBJDIR)$(1)/
$(1)_depdir := $(DEPDIR)$(1)/
$(1)_objs := $$($(1)_c_sources:%.c=$$($(1)_objdir)%.o) $$($(1)_s_sources:%.s=$$($(1)_objdir)%.o)

$(BINDIR)$(1): $$($(1)_objs) $$($(1)_EXTRA_DEPS) $$(makefile)
	@mkdir -p $$(dir $$@)
	@echo "LD    $$@"
	@$(LD) $$($(1)_objs) -o $$@ $$($(1)_LDFLAGS)

$$($(1)_objdir)%.o: $(CWD)%.c $$(makefile)
	@mkdir -p $$(dir $$@)
	@mkdir -p $$($(1)_depdir)
	@echo "CC    $$<"
	@$(GCC) $$($(1)_CFLAGS) -MP -MD -MF $$(<:$(CWD)%.c=$$($(1)_depdir)%.d) -c -o $$@ $$<
	
$$($(1)_objdir)%.o: $(CWD)%.s $$(makefile)
	@mkdir -p $$(dir $$@)
	@echo "AS    $$<"
	@$(AS) $$($(1)_AFLAGS) -o $$@ $$<

ALL_TARGETS += $(BINDIR)$(1)
ALL_DEPS += $$($(1)_c_sources:%.c=$$($(1)_depdir)%.d)
endef

all: all_internal

define do_include
makefile := $(1)
CWD := $$(dir $$(makefile))
ifeq ($$(CWD),./)
  CWD :=
endif
SUBDIRS :=
TARGETS :=
include $(1)
$$(foreach target,$$(TARGETS),$$(eval $$(call build,$$(target))))
$$(foreach subdir,$$(SUBDIRS),$$(eval $$(call do_include,$$(CWD)$$(subdir)/make.mk)))
endef

$(eval $(call do_include,make.mk))

all_internal: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
rebuild: clean all

-include $(ALL_DEPS)
