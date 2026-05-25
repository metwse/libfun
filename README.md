# libfun
C utilities written for fun.

*Check out the [online documentation](https://metwse.github.io/libfun/) for
public API details!*


## Quick Integration
You can easily embed libfun build system inside your own Makefiles by including
the [libfun.mk](./libfun.mk).

```makefile
# Set the path to your libfun source folder. (REQUIRED)
LIBFUN_DIR := vendor/libfun

# Optionally configure the build variables
LIBFUN_MODE := debug
LIBFUN_PREFIX := lf_

# You may prefer installing libfun using git
$(LIBFUN_DIR)/libfun.mk:
	git clone https://github.com/metwse/libfun.git $(LIBFUN_DIR)

# Include the libfun build system
include $(LIBFUN_DIR)/libfun.mk

# ...

# Use the exported variables in your targets. $(LIBFUN) points to the static
# library path.
my_app: main.c $(LIBFUN)
	$(CC) -I$(LIBFUN_INCLUDE_DIR) $< $(LIBFUN) -o $@
```

### Configuration Variables
libfun provides an include-based build which uses the following `LIBFUN_*`
configuration variables to control the build process.

| Variable | Description | Default | Valid Values |
|----------|-------------|---------|--------------|
| `LIBFUN_MODE` | Determines the optimization level and instrumentation. | `release` | `release`, `debug`, `test` |
| `LIBFUN_PREFIX` | Symbol prefix for public functions and structs. | `f` | any C identifier |
| `LIBFUN_DIR` | Path to the root of the libfun source repository. | `.` (*do not* use the default) | libfun path |

`libfun.mk` defines three target variables: `LIBFUN`, the static library target,
`LIBFUN_SO`, the shared object version and `LIBFUN_H`, the header-only library.
You can set these before including the `libfun.mk` file to output into desired
location, but you can also use the default values that output to libfun's
internal build directory.

A variable named `LIBFUN_INCLUDE_DIR` is also defined to point to the folder
containing the public headers.


## `contribute -Wai-slop`
<img width="96" height="96" alt="no-ai-slop" align="right" src="https://github.com/user-attachments/assets/bca16d5a-a6fe-4cbf-b41f-1176e000cff2" />

Contributions are welcome! Please check our
[Code of Conduct](http://github.com/metwse/code-of-conduct) before submitting
pull requests.

