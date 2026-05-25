# Currenlty only integration tests exist, but the Makefile designed for future
# extensions for different test types, i.e. fuzz test, unit test.

INTEGRATION_DIR = integration

DIST_DIR = ../dist/tests
OBJ_DIR = $(DIST_DIR)/obj

# No need to change rules below this line.

CFLAGS = -std=c11 -Wall -Wextra -pedantic -O0 -g3 --coverage -DLIBFUN_PREFIX=$(LIBFUN_PREFIX)

INTEGRATION_SRCS = $(wildcard $(INTEGRATION_DIR)/*.c)

TEST_TARGETS = \
	$(patsubst $(INTEGRATION_DIR)/%.c, $(DIST_DIR)/%.integration.test, $(INTEGRATION_SRCS))

OBJS = $(wildcard $(OBJ_DIR)/*.o)


default: $(TEST_TARGETS)

PREFIX_FILE = .prefix

LIBFUN_DIR := ..
LIBFUN_MODE := test
LIBFUN_PREFIX := $(shell \
		if [ ! -f $(PREFIX_FILE) ]; then \
			 echo t$$RANDOM >> $(PREFIX_FILE); \
		fi; \
		cat $(PREFIX_FILE))

include ../libfun.mk


.SECONDARY:
$(OBJ_DIR)/%.integration.test.o: $(INTEGRATION_DIR)/%.c | $(OBJ_DIR)
	cd ..; $(CC) $(CFLAGS) -c tests/$< -o tests/$@
	$(CC) -MM $< -MF $(@:.o=.d) -MT $@

$(DIST_DIR)/%.integration.test: $(OBJ_DIR)/%.integration.test.o $(LIBFUN) \
		| $(DIST_DIR)
	$(CC) $(CFLAGS) $^ -o $@


$(DIST_DIR) $(OBJ_DIR):
	mkdir -p $@


-include $(OBJS:.o=.d)

.PHONY: default
