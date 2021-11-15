include defines.mk product.mk

CPPFLAGS = -I dmklib '-DVERSION="$(VERSION)"'
CFLAGS = -Wall -Werror -Wfatal-errors

build_dir     ?= build
bin_target    ?= $(PRODUCT)
build_target  ?= $(build_dir)/$(bin_target)
build_targets ?= $(build_target)

obj_targets = dmklib/libdmk.o $(PRODUCT).o

tar_files   = LICENSE README.md $(build_targets)

libdmk_args = SOURCES=libdmk.c TARGETS=libdmk.o CC=$(CC)

clean_files     = $(addprefix $(build_dir)/,$(bin_target) $(obj_targets))
clobber_files   = $(clean_files) $(build_dir)
distclean_files = $(clobber_files) $(wildcard build.*)

clean_files     = $(BINS)
clobber_files   = $(clean_files) $(OBJS)
distclean_files = $(clobber_files)

all: $(build_target)

$(build_target): FORCE | $(build_dir)
	$(MAKE) -C '$(@D)' -I .. -f ../Makefile '$(@F)'

$(bin_target): $(obj_targets) | dmklib
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(PRODUCT).c: ../$(PRODUCT).c
	ln -s -- '$<' '$@'

dmklib/libdmk.o: dmklib/libdmk.c dmklib/libdmk.h
	$(MAKE) -C dmklib $(libdmk_args)

dmklib/libdmk.c dmklib/libdmk.h: | dmklib

dmklib:
	rm -rf -- '$@'
	cp -rp -- '../$@' .
	$(MAKE) -C dmklib $(libdmk_args) clean

$(build_dir):
	mkdir -p -- '$@'

release: $(TARBALLGZ)

$(TARBALLGZ): $(tar_files)
	tar --transform='s:^build[/.]::' -czf '$@' $^

clean clobber distclean:
	$(call scrub_files_call,$($@_files))
	$(MAKE) -C dmklib $(libdmk_args) clean

.PHONY: all release clean clobber distclean FORCE
.DELETE_ON_ERROR:
