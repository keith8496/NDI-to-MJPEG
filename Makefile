ifeq ($(NDILIB),)
  CC     ?= gcc
  NDILIB := $(shell perl -e '@_=split/-/,"$(shell $(CC) -dumpmachine)"; $$"="-"; if (scalar @_ > 3) { print "@_[0,2,3]" } else { print "@_" }')
endif

BMDSDK := NDIlib_Send_BMD/BMDSDK/Linux

PROJECTS := \
	ndiFind

.PHONY: all
all:
	@if [ ! -f ./lib/libndi.so ] && [ ! -f ./lib/$(NDILIB)/libndi.so ]; then \
		echo libndi.so not found: $(NDILIB) >&2; \
		false; \
	fi
	-@for proj in $(PROJECTS); do \
		if [ -d $$proj ]; then \
			if [ -f ../../lib/libndi.so ]; then \
				$(MAKE) -C $$proj -f ../Makefile.proj NDILIB=; \
			else \
				$(MAKE) -C $$proj -f ../Makefile.proj NDILIB=$(NDILIB); \
			fi \
		fi \
	done

.PHONY: clean
clean:
	-@for proj in $(PROJECTS); do \
		if [ -d $$proj ]; then \
			$(MAKE) -C $$proj -f ../Makefile.proj clean; \
		fi \
	done
