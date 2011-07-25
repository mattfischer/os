CROSS_COMPILE := arm-none-eabi

GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb

OBJDIR := $(OUTDIR)obj/
BINDIR := $(OUTDIR)bin/
DEPDIR := $(OUTDIR)deps/

define build_target
target := $(1)
bindir := $$(BINDIR)
objdir := $$(OBJDIR)$$(target)
depdir := $$(DEPDIR)$$(target)
sources := $$($$(target)_SOURCES)
cflags := $$($$(target)_CFLAGS)
aflags := $$($$(target)_AFLAGS)
ldflags := $$($$(target)_LDFLAGS)
extra_deps := $$($$(target)_EXTRA_DEPS)

c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)
binary := $$(bindir)$$(target)

$$(binary): objects := $$(objects)
$$(binary): extra_deps := $$(extra_deps)
$$(binary): makefile := $$(makefile)
$$(binary): bindir := $$(bindir)
$$(binary): ldflags := $$(ldflags)

$$(objdir)/%.o: CWD := $$(CWD)
$$(objdir)/%.o: makefile := $$(makefile)
$$(objdir)/%.o: objdir := $$(objdir)
$$(objdir)/%.o: depdir := $$(depdir)
$$(objdir)/%.o: cflags := $$(cflags)
$$(objdir)/%.o: aflags := $$(aflags)

$$(binary): $$(objects) $$(extra_deps) $$(makefile)
	@echo "LD      $$@"
	@mkdir -p $$(bindir)
	@$$(LD) $$(objects) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "CC      $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(GCC) $$(cflags) -MP -MD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<

$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "AS      $$<"
	@mkdir -p $$(objdir)
	@$$(AS) $$(aflags) -o $$@ $$<

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
endef
