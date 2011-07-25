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
target := $(1)
objdir := $$(OBJDIR)$$(target)
depdir := $$(OBJDIR)$$(target)
sources := $$($$(target)_SOURCES)
cflags := $$($$(target)_CFLAGS)
aflags := $$($$(target)_AFLAGS)
ldflags := $$($$(target)_LDFLAGS)
extra_deps := $$($$(target)_EXTRA_DEPS)

c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)
binary := $$(BINDIR)$$(target)

$$(binary): $$(objects) $$(extra_deps) $$(makefile)
	@echo "LD    $$@"
	@mkdir -p $$(BINDIR)
	@$$(LD) $$(objects) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "CC    $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(GCC) $$(cflags) -MP -MD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<
	
$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "AS    $$<"
	@mkdir -p $$(objdir)
	@$$(AS) $$(aflags) -o $$@ $$<

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
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
