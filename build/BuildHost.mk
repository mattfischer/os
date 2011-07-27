HOST_GCC := gcc
HOST_AS := as
HOST_LD := gcc

HOST_OUTDIR := $(OUTDIR)host/
HOST_OBJDIR := $(HOST_OUTDIR)obj/
HOST_BINDIR := $(HOST_OUTDIR)bin/
HOST_DEPDIR := $(HOST_OUTDIR)deps/

HOST_CFLAGS := -I .

HOST_EXE_EXT := .exe

define build_host
target := $(1)
bindir := $$(HOST_BINDIR)
objdir := $$(HOST_OBJDIR)$$(target)
depdir := $$(HOST_DEPDIR)$$(target)
sources := $$($$(target)_SOURCES)
cflags := $$($$(target)_CFLAGS)
aflags := $$($$(target)_AFLAGS)
ldflags := $$($$(target)_LDFLAGS)
extra_deps := $$($$(target)_EXTRA_DEPS)

c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)
binary := $$(bindir)$$(target)$$(HOST_EXE_EXT)

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

clean-$$(target): binary := $$(binary)
clean-$$(target): objdir := $$(objdir)
clean-$$(target): depdir := $$(depdir)

$$(binary): $$(objects) $$(extra_deps) $$(makefile)
	@echo "HOST_LD   $$@"
	@mkdir -p $$(bindir)
	@$$(HOST_LD) $$(objects) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "HOST_CC   $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(HOST_GCC) $$(cflags) $$(HOST_CFLAGS) -MP -MD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<
	
$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "HOST_AS   $$<"
	@mkdir -p $$(objdir)
	@$$(HOST_AS) $$(aflags) -o $$@ $$<

clean-$$(target):
	@echo "Cleaning target '$$(target)'"
	@rm $$(binary)
	@rm -rf $$(objdir)
	@rm -rf $$(depdir)

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
endef

