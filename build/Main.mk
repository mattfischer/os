OUTDIR := out/

include $(BUILD_DIR)BuildBinary.mk

all: build_all

define do_include
makefile := $(1)
cwdstack := $$(CWD) $$(cwdstack)
CWD := $$(dir $$(makefile))
ifeq ($$(CWD),./)
  CWD :=
endif

SUBDIRS :=
BINARIES :=

include $$(makefile)
$$(foreach target,$$(BINARIES),$$(eval $$(call build_binary,$$(target))))
$$(foreach subdir,$$(SUBDIRS),$$(eval $$(call do_include,$$(CWD)$$(subdir)/Build.mk)))
CWD := $$(firstword $$(cwdstack))
ifeq ($$(CWD),./)
  CWD :=
endif

cwdstack := $$(wordlist 2,$$(words $$(cwdstack)),$$(cwdstack))
endef

ALL_TARGETS :=
ALL_DEPS :=

$(eval $(call do_include,Build.mk))

build_all: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
rebuild: clean all

-include $(ALL_DEPS)
