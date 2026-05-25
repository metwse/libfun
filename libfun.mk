# libfun build mode
LIBFUN_MODE ?= release

# Symbol prefix (for functions and types).
LIBFUN_PREFIX ?= f

# The directory containing libfun repository
LIBFUN_DIR ?= .

# libfun public headers location
LIBFUN_INCLUDE_DIR := $(LIBFUN_DIR)/include

# private
libfun_DIST_DIR := $(LIBFUN_DIR)/dist
libfun_TARGET_DIR := $(libfun_DIST_DIR)/$(LIBFUN_MODE).$(LIBFUN_PREFIX)

# Build outputs.
LIBFUN_SO ?= $(libfun_TARGET_DIR)/libfun.so
LIBFUN ?= $(libfun_TARGET_DIR)/libfun.a
LIBFUN_H ?= $(libfun_DIST_DIR)/libfun.h


# Variables below this line are private.
# -----------------------------------------------------------------------------
libfun_CFLAGS_COMMON := -std=c11 -Wall -Wextra -pedantic -fPIC -DLIBFUN_PREFIX=$(LIBFUN_PREFIX)

libfun_CFLAGS_release := $(libfun_CFLAGS_COMMON) -O3 -flto
libfun_CFLAGS_debug := $(libfun_CFLAGS_COMMON) -O0 -g3
libfun_CFLAGS_test := $(libfun_CFLAGS_COMMON) -O0 -g3 --coverage

libfun_CFLAGS := $(libfun_CFLAGS_$(LIBFUN_MODE))

ifeq ($(libfun_CFLAGS),)
$(error "WARNING: unknown mode $(LIBFUN_MODE).")
endif

libfun_HEADERS_TOPOLOGICAL_ORDERED = config.h stack.h map.h

libfun_SRC_DIR := $(LIBFUN_DIR)/src

libfun_OBJ_DIR := $(libfun_TARGET_DIR)/obj

libfun_MKDIR := $(or $(MKDIR),mkdir -p)

libfun_HEADERS := $(addprefix $(LIBFUN_INCLUDE_DIR)/,\
		    $(libfun_HEADERS_TOPOLOGICAL_ORDERED))

libfun_SRCS := $(wildcard $(libfun_SRC_DIR)/*.c)
libfun_OBJS := $(patsubst $(libfun_SRC_DIR)/%.c,\
		 $(libfun_OBJ_DIR)/%.o,\
		 $(libfun_SRCS))


$(LIBFUN_H): $(libfun_HEADERS) $(libfun_SRCS) | $(libfun_DIST_DIR)
	echo '#define LF_HEADERONLY' > $(LIBFUN_H)
	cat $(libfun_HEADERS) >> $(LIBFUN_H)
	echo '#ifdef LF_IMPLEMENTATION' >> $(LIBFUN_H)
	cat $(libfun_SRCS) >> $(LIBFUN_H)
	echo '#endif' >> $(LIBFUN_H)

$(LIBFUN) $(LIBFUN_SO): CFLAGS := $(libfun_CFLAGS)

$(LIBFUN): $(libfun_OBJS) | $(libfun_DIST_DIR)
	$(AR) rcs $@ $^

$(LIBFUN_SO): $(libfun_OBJS) | $(libfun_DIST_DIR)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(libfun_OBJ_DIR)/%.o: $(libfun_SRC_DIR)/%.c | $(libfun_OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(libfun_OBJ_DIR) $(libfun_DIST_DIR):
	$(libfun_MKDIR) $@


-include $(libfun_OBJS:.o=.d)
