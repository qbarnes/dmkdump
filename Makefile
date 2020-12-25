define nl
 
  
endef

scrub_files_call = $(foreach f,$(wildcard $(1)),$(RM) -r -- '$f'$(nl))

CFLAGS = -Wall -Werror -Wfatal-errors
#CFLAGS = -Wall

BINS = dmkdump
OBJS = dmkdump.o libdmk.o

clean_files     = $(BINS)
clobber_files   = $(clean_files) $(OBJS)
distclean_files = $(clobber_files)

all: $(BINS)

dmkdump: dmkdump.o libdmk.o

clean clobber distclean:
	$(call scrub_files_call,$($@_files))

.PHONY: all clean clobber distclean
.DELETE_ON_ERROR:
