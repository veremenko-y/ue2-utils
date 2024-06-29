include ../../common.mk

CFLAGS += -I../../include

.PHONY: all

all: ue2-as

ue2-as: as0.o as1.o as2.o as.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ue2-as *.o *.out out.*
