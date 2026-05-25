RM = rm -rf
MODE ?= release


default: _default


LIBFUN_MODE := $(MODE)

include libfun.mk

_default: $(LIBFUN) $(LIBFUN_SO)


clean:
	$(RM) $(libfun_DIST_DIR) docs-autogen

docs:
	doxygen


.PHONY: default _default clean docs
