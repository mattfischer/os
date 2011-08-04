CROSS_COMPILE := arm-none-eabi
CROSS_GCC := $(CROSS_COMPILE)-gcc
CROSS_AS := $(CROSS_COMPILE)-as
CROSS_LD := $(CROSS_COMPILE)-ld
CROSS_GDB := $(CROSS_COMPILE)-gdb
CROSS_OBJCOPY := $(CROSS_COMPILE)-objcopy
HOST_GCC := gcc
HOST_AS := as
HOST_LD := gcc

CROSS_OBJDIR := $(OUTDIR)obj/
CROSS_BINDIR := $(OUTDIR)bin/
CROSS_DEPDIR := $(OUTDIR)deps/
HOST_OUTDIR  := $(OUTDIR)host/
HOST_OBJDIR  := $(HOST_OUTDIR)obj/
HOST_BINDIR  := $(HOST_OUTDIR)bin/
HOST_DEPDIR  := $(HOST_OUTDIR)deps/

HOST_CFLAGS := -I . -g
CROSS_CFLAGS := -I . -g

HOST_EXE_EXT := .exe

define build_binary
target := $(1)

ifeq ($$($$(target)_PLATFORM),HOST)
  binary := $$(HOST_BINDIR)$$(target)$$(HOST_EXE_EXT)
  $$(binary): cc := $(HOST_GCC)
  $$(binary): cc_desc := HOST_CC  #
  $$(binary): as := $(HOST_AS)
  $$(binary): as_desc := HOST_AS  #
  $$(binary): ld := $(HOST_LD)
  $$(binary): ld_desc := HOST_LD  #

  bindir := $$(HOST_BINDIR)
  objdir := $$(HOST_OBJDIR)$$(target)
  depdir := $$(HOST_DEPDIR)$$(target)
  global_cflags := $(HOST_CFLAGS)
  global_aflags := $(HOST_AFLAGS)
  global_ldflags := $(HOST_LDFLAGS)
else
  binary := $$(CROSS_BINDIR)$$(target)
  $$(binary): cc := $(CROSS_GCC)
  $$(binary): cc_desc := CC       #
  $$(binary): as := $(CROSS_AS)
  $$(binary): as_desc := AS       #
  $$(binary): ld := $(CROSS_LD)
  $$(binary): ld_desc := LD       #

  bindir := $$(CROSS_BINDIR)
  objdir := $$(CROSS_OBJDIR)$$(target)
  depdir := $$(CROSS_DEPDIR)$$(target)
  global_cflags := $(CROSS_CFLAGS)
  global_aflags := $(CROSS_AFLAGS)
  global_ldflags := $(CROSS_LDFLAGS)
endif

sources := $$($$(target)_SOURCES)
c_sources := $$(filter %.c,$$(sources))
s_sources := $$(filter %.s,$$(sources))
objects := $$(c_sources:%.c=$$(objdir)/%.o) $$(s_sources:%.s=$$(objdir)/%.o)

$$(binary): CWD := $$(CWD)
$$(binary): makefile := $$(makefile)
$$(binary): cflags := $$(global_cflags) $$($$(target)_CFLAGS)
$$(binary): aflags := $$(global_aflags) $$($$(target)_AFLAGS)
$$(binary): ldflags := $$(global_ldflags) $$($$(target)_LDFLAGS)
$$(binary): objects := $$(objects)
$$(binary): extra_deps := $$($$(target)_EXTRA_DEPS)
$$(binary): extra_objs := $$($$(target)_EXTRA_OBJS)
$$(binary): bindir := $$(bindir)
$$(binary): objdir := $$(objdir)
$$(binary): depdir := $$(depdir)

extra_objs := $$($$(target)_EXTRA_OBJS)
extra_deps := $$($$(target)_EXTRA_DEPS)

clean-$$(target): target := $$(target)
clean-$$(target): binary := $$(binary)
clean-$$(target): objdir := $$(objdir)
clean-$$(target): depdir := $$(depdir)

$$(target): $$(binary)

$$(binary): $$(objects) $$(extra_objs) $$(extra_deps) $$(makefile)
	@echo "  $$(ld_desc) $$@"
	@mkdir -p $$(bindir)
	@$$(ld) $$(objects) $$(extra_objs) -o $$@ $$(ldflags)

$$(objdir)/%.o: $$(CWD)%.c $$(makefile)
	@echo "  $$(cc_desc) $$<"
	@mkdir -p $$(objdir)
	@mkdir -p $$(depdir)
	@$$(cc) $$(cflags) -MP -MMD -MF $$(<:$$(CWD)%.c=$$(depdir)/%.d) -c -o $$@ $$<

$$(objdir)/%.o: $$(CWD)%.s $$(makefile)
	@echo "  $$(as_desc) $$<"
	@mkdir -p $$(objdir)
	@$$(as) $$(aflags) -o $$@ $$<

clean-$$(target):
	@echo "Cleaning target '$$(target)'"
	@rm $$(binary)
	@rm -rf $$(objdir)
	@rm -rf $$(depdir)

ALL_TARGETS += $$(binary)
ALL_DEPS += $$(c_sources:%.c=$$(depdir)/%.d)
endef
