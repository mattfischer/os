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

$(BINDIR)$(1): $$($(1)_objs)
	@mkdir -p $$(dir $$@)
	@$(LD) $$($(1)_objs) -o $$@ $$($(1)_LDFLAGS)
	@echo "LD    $$@"

$$($(1)_objdir)%.o: $(cwd)%.c
	@mkdir -p $$(dir $$@)
	@mkdir -p $$($(1)_depdir)
	@$(GCC) $$($(1)_CFLAGS) -MP -MD -MF $$(<:$(cwd)%.c=$$($(1)_depdir)%.d) -c -o $$@ $$<
	@echo "CC    $$<"
	
$$($(1)_objdir)%.o: $(cwd)%.s
	@mkdir -p $$(dir $$@)
	@$(AS) $(AFLAGS) -o $$@ $$<
	@echo "AS    $$<"

ALL_TARGETS += $(BINDIR)$(1)
ALL_DEPS += $$($(1)_c_sources:$(cwd)%.c=$$($(1)_depdir)%.d)
endef

all: all_internal

define do_include
cwd := $$(dir $(1))
ifeq ($$(cwd),./)
  cwd :=
endif
SUBDIRS :=
TARGETS :=
include $(1)
$$(foreach target,$$(TARGETS),$$(eval $$(call build,$$(target))))
$$(foreach subdir,$$(SUBDIRS),$$(eval $$(call do_include,$$(cwd)$$(subdir)/make.mk)))
endef

$(eval $(call do_include,make.mk))

all_internal: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
-include $(ALL_DEPS)
