include ../common.mk

CFLAGS += -I../include

.PHONY: all

PROGRAMS=nm size


all: $(PROGRAMS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

clean:
	rm -f $(PROGRAMS)
