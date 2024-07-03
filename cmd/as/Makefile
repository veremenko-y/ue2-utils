include ../../common.mk

CFLAGS += -I../../include

.PHONY: all test

all: ue2-as

test: all
	./ue2-as hellorld.s -o hellorld.out && \
	./ue2-as def.s -o def.out

ue2-as: as0.o as1.o as2.o as.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ue2-as *.o *.out out.*
