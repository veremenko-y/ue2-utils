export PATH := $(PWD)/bin:$(PATH)

.PHONY: all clean
.SUFFIXES:

KERNEL = sys/entry sys/blk sys/fs sys/mem sys/proc sys/math sys/sysent sys/trap sys/exit
# OUT=$(BIN_DIR)/$(call out_name,as)

all: hellorld.bin
#all: boot standalone hellorld.bin

# kernel.bin: $(KERNEL:%=%.o)
# 	$(CC) $(LDFLAGS) $^.asm $(LDLIBS) -o $@

standalone: standalone/entry.o

hellorld.bin: hellorld.s
	ue2-as hellorld.s -o hellorld.o
	ue2-ld -b -o hellorld.bin -L hellorld.o

boot: boot.o
	ue2-ld -b -o boot.bin boot.o

%.o: %.asm 
	ue2-cpp -I. $< -o $<.s
	ue2-as $<.s -o $@ && rm $<.s
#	ue2-nm $@

clean:
	rm -f *.o *.asm.s
	rm -f standalone/*.o standalone/*.asm.s
