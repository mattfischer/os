CROSS_COMPILE := arm-none-eabi

GCC := $(CROSS_COMPILE)-gcc
AS := $(CROSS_COMPILE)-as
LD := $(CROSS_COMPILE)-ld
GDB := $(CROSS_COMPILE)-gdb
OBJCOPY := $(CROSS_COMPILE)-objcopy

OBJDIR := $(OUTDIR)obj/
BINDIR := $(OUTDIR)bin/
DEPDIR := $(OUTDIR)deps/

CFLAGS := -I .

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
extra_objs := $$($$(target)_EXTRA_OBJS)

c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)
binary := $$(bindir)$$(target)

$$(binary): objects := $$(objects)
$$(binary): extra_deps := $$(extra_deps)
$$(binary): makefile := $$(makefile)
$$(binary): bindir := $$(bindir)
$$(binary): ldflags := $$(ldflags)
$$(binary): extra_objs := $$(extra_objs)

$$(objdir)/%.o: CWD := $$(CWD)
$$(objdir)/%.o: makefile := $$(makefile)
$$(objdir)/%.o: objdir := $$(objdir)
$$(objdir)/%.o: depdir := $$(depdir)
$$(objdir)/%.o: cflags := $$(cflags)
$$(objdir)/%.o: aflags := $$(aflags)

clean-$$(target): target := $$(target)
clean-$$(target): binary := $$(binary)
clean-$$(target): objdir := $$(objdir)
clean-$$(target): depdir := $$(depdir)

$$(binary): $$(objects) $$(extra_objs) $$(extra_deps) $$(makefile)
	@echo "LD        $$@"
	@mkdir -p $$(bindir)
	@$$(LD) $$(objects) $$(extra_objs) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "CC        $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(GCC) $$(cflags) $$(CFLAGS) -MP -MMD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<

$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "AS        $$<"
	@mkdir -p $$(objdir)
	@$$(AS) $$(aflags) -o $$@ $$<

clean-$$(target):
	@echo "Cleaning target '$$(target)'"
	@rm $$(binary)
	@rm -rf $$(objdir)
	@rm -rf $$(depdir)

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
endef
