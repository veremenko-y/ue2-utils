include $(PWD)/common.mk

SRC = cpp.c hash.c main.c token1.c token2.c

.SUFFIXES: .c .o
OBJ = $(SRC:.c=.o)
APP=cpp
OUT=$(BIN_DIR)/$(call out_name,$(APP))

all: $(OUT)

.c.o:
	$(CC) $(CFLAGS) $(CCOPTS) -c $<

$(OUT): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@ 

clean:
	rm -f *.o $(APPS) $(APPSNS) $(SRCS:.c=) core *~ *.asm *.lst *.sym *.map *.noi *.lk *.ihx *.tmp *.bin size.report

rmbak:
	rm -f *~ core

main.o: cc.h
cpp.o: cc.h
hash.o: cc.h
tree.o: cc.h

token1.o: token1.h
token2.o: token2.h
