include ../common.mk

CFLAGS += -I../include

.PHONY: all

PROGRAMS=nm size ld dis


all: $(PROGRAMS:%=ue2-%)

ue2-%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

clean:
	rm -f $(PROGRAMS:%=ue2-%)
	rm -f *.out
