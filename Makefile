

export CC := gcc

export CFLAGS =

export BIN_DIR = $(PWD)/bin
export TMP_DIR = $(PWD)/tmp
export DESTDIR = /opt/ue2
export PREFIX = ue2-


ifeq ($(OS),Windows_NT)

export PWD=$(CURDIR)
export SUFIX=.exe

else

export ARCH=-m32
export PWD=$(shell pwd)

endif

#RELEASE=1

ifdef RELEASE
CFLAGS +=-Os -s
else
CFLAGS +=-Og -g -ggdb
endif
CFLAGS += -fno-pie -no-pie # not to make a position independent executable (PIE)
CFLAGS += -fno-ident # GCC outputs an entire section to advertise itself, which, once padded/aligned is significant in a small program
DISWARN = -Wno-int-conversion -Wno-incompatible-pointer-types # Disable specific warnings, because old Unix code
CFLAGS += -fdiagnostics-color=always -std=c90 $(ARCH) $(DISWARN) 
export LDFLAGS = $(ARCH)

.PHONY: all clean test

all:
	mkdir bin
	make -C cmd -f build.mk
	make -C cmd/as -f build.mk

clean:
	rm -f bin/*

test: all
	mkdir tmp
	./bin/ue2-as test/hellorld.s -o $(TMP_DIR)/hellorld.out
	./bin/ue2-as test/def.s -o $(TMP_DIR)/def.out
	./bin/ue2-ld -o $(TMP_DIR)/test1.out $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
	./bin/ue2-ld -s -o $(TMP_DIR)/test1-strip.out $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
	./bin/ue2-ld -x -o $(TMP_DIR)/test1-ext.out $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
	./bin/ue2-ld -b -o $(TMP_DIR)/test1.bin $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
	
	./bin/ue2-size $(TMP_DIR)/*.out
	./bin/ue2-nm $(TMP_DIR)/*.out
	./bin/ue2-hdr $(TMP_DIR)/*.out