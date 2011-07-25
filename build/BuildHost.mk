HOST_GCC := gcc
HOST_AS := as
HOST_LD := ld

HOST_OUTDIR := $(OUTDIR)host/
HOST_OBJDIR := $(HOST_OUTDIR)obj/
HOST_BINDIR := $(HOST_OUTDIR)bin/
HOST_DEPDIR := $(HOST_OUTDIR)deps/

define build_host
target := $(1)
bindir := $$(HOST_BINDIR)
objdir := $$(HOST_OBJDIR)$$(target)
depdir := $$(HOST_OBJDIR)$$(target)
sources := $$($$(target)_SOURCES)
cflags := $$($$(target)_CFLAGS)
aflags := $$($$(target)_AFLAGS)
ldflags := $$($$(target)_LDFLAGS)
extra_deps := $$($$(target)_EXTRA_DEPS)

c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)
binary := $$(bindir)$$(target)

$$(binary): $$(objects) $$(extra_deps) $$(makefile)
	@echo "HOST_LD $$@"
	@mkdir -p $$(bindir)
	@$$(LD) $$(objects) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "HOST_CC $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(GCC) $$(cflags) -MP -MD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<
	
$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "HOST_AS $$<"
	@mkdir -p $$(objdir)
	@$$(AS) $$(aflags) -o $$@ $$<

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
endef

