OUTDIR := out/

include $(BUILD_DIR)BuildTarget.mk
include $(BUILD_DIR)BuildHost.mk

all: build_all

define do_include
makefile := $(1)
cwdstack := $$(CWD) $$(cwdstack)
CWD := $$(dir $$(makefile))
ifeq ($$(CWD),./)
  CWD :=
endif

SUBDIRS :=
TARGETS :=
HOST_TARGETS :=

include $$(makefile)
$$(foreach target,$$(TARGETS),$$(eval $$(call build_target,$$(target))))
$$(foreach target,$$(HOST_TARGETS),$$(eval $$(call build_host,$$(target))))
$$(foreach subdir,$$(SUBDIRS),$$(eval $$(call do_include,$$(CWD)$$(subdir)/Build.mk)))
CWD := $$(firstword $$(cwdstack))
ifeq ($$(CWD),./)
  CWD :=
endif

cwdstack := $$(wordlist 2,$$(words $$(cwdstack)),$$(cwdstack))
endef

ALL_TARGETS :=

$(eval $(call do_include,Build.mk))

build_all: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
rebuild: clean all

-include $(ALL_DEPS)
