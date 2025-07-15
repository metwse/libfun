CFLAGS = -Wall -Werror -std=gnu18
BUILDDIR = dist

TOOLS = factorization counter

$(TOOLS): $(addsuffix .c, $@) | $(BUILDDIR)
	cc $(CFLAGS) $@.c -o $(BUILDDIR)/$@ -lm

$(BUILDDIR):
	mkdir $(BUILDDIR)

clean:
	rm -r $(BUILDDIR)

.PHONY = clean
