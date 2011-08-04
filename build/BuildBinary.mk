define build_binary
target := $(1)

ifeq ($$($$(target)_PLATFORM),HOST)
  ifeq ($$($$(target)_TYPE),LIBRARY)
    binary := $$(HOST_LIBDIR)$$(target)$$(HOST_LIB_EXT)
	outdir := $$(HOST_LIBDIR)
  else
    binary := $$(HOST_BINDIR)$$(target)$$(HOST_EXE_EXT)
	outdir := $$(HOST_BINDIR)
  endif

  $$(binary): cc := $(HOST_GCC)
  $$(binary): cc_desc := HOST_CC  #
  $$(binary): as := $(HOST_AS)
  $$(binary): as_desc := HOST_AS  #
  $$(binary): ld := $(HOST_LD)
  $$(binary): ld_desc := HOST_LD  #
  $$(binary): ar := $(HOST_AR)
  $$(binary): ar_desc := HOST_AR  #

  objdir := $$(HOST_OBJDIR)$$(target)
  depdir := $$(HOST_DEPDIR)$$(target)
  global_cflags := $(HOST_CFLAGS)
  global_aflags := $(HOST_AFLAGS)
  global_ldflags := $(HOST_LDFLAGS)
  libs := $$($$(target)_LIBS:%=$$(HOST_LIBDIR)%$$(HOST_LIB_EXT))
else
  ifeq ($$($$(target)_TYPE),LIBRARY)
    binary := $$(CROSS_LIBDIR)$$(target)$$(CROSS_LIB_EXT)
	outdir := $$(CROSS_LIBDIR)
  else
    binary := $$(CROSS_BINDIR)$$(target)$$(CROSS_EXE_EXT)
	outdir := $$(CROSS_BINDIR)
  endif

  $$(binary): cc := $(CROSS_GCC)
  $$(binary): cc_desc := CC       #
  $$(binary): as := $(CROSS_AS)
  $$(binary): as_desc := AS       #
  $$(binary): ld := $(CROSS_LD)
  $$(binary): ld_desc := LD       #
  $$(binary): ar := $(CROSS_AR)
  $$(binary): ar_desc := AR       #

  objdir := $$(CROSS_OBJDIR)$$(target)
  depdir := $$(CROSS_DEPDIR)$$(target)
  global_cflags := $(CROSS_CFLAGS)
  global_aflags := $(CROSS_AFLAGS)
  global_ldflags := $(CROSS_LDFLAGS)
  libs := $$($$(target)_LIBS:%=$$(CROSS_LIBDIR)%$$(CROSS_LIB_EXT))
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
$$(binary): libs := $$(libs)
$$(binary): extra_deps := $$($$(target)_EXTRA_DEPS)
$$(binary): extra_objs := $$($$(target)_EXTRA_OBJS)
$$(binary): outdir := $$(outdir)
$$(binary): objdir := $$(objdir)
$$(binary): depdir := $$(depdir)

extra_objs := $$($$(target)_EXTRA_OBJS)
extra_deps := $$($$(target)_EXTRA_DEPS)

clean-$$(target): target := $$(target)
clean-$$(target): binary := $$(binary)
clean-$$(target): objdir := $$(objdir)
clean-$$(target): depdir := $$(depdir)

$$(target): $$(binary)

ifeq ($$($$(target)_TYPE),LIBRARY)
$$(binary): $$(objects) $$(extra_objs) $$(extra_deps) $$(makefile)
	@echo "  $$(ar_desc) $$@"
	@mkdir -p $$(outdir)
	@$$(ar) cr $$@ $$(objects) $$(extra_objs) 1>/dev/null
else
$$(binary): $$(objects) $$(libs) $$(extra_objs) $$(extra_deps) $$(makefile)
	@echo "  $$(ld_desc) $$@"
	@mkdir -p $$(outdir)
	@$$(ld) $$(objects) $$(libs) $$(extra_objs) -o $$@ $$(ldflags)
endif

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
