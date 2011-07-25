OUTDIR := out/

include $(BUILD_DIR)BuildTarget.mk
include $(BUILD_DIR)BuildHost.mk

all: build_all

define do_include
makefile := $(1)
CWD := $$(dir $$(makefile))
ifeq ($$(CWD),./)
  CWD :=
endif
SUBDIRS :=
TARGETS :=
HOST_TARGETS :=
include $(1)
$$(foreach target,$$(TARGETS),$$(eval $$(call build_target,$$(target))))
$$(foreach target,$$(HOST_TARGETS),$$(eval $$(call build_host,$$(target))))
$$(foreach subdir,$$(SUBDIRS),$$(eval $$(call do_include,$$(CWD)$$(subdir)/Build.mk)))
endef

$(eval $(call do_include,Build.mk))

build_all: $(ALL_TARGETS)

clean:
	@rm -rf $(OUTDIR)
	@echo "Build directory removed"
	
rebuild: clean all

-include $(ALL_DEPS)
