.SUFFIXES:
.PHONY: all clean test

export BITS12
#export BITS16

#export CC := gcc -v
export CC := gcc

export CFLAGS =

export BIN_DIR = $(PWD)/bin
export TMP_DIR = $(PWD)/tmp
export DESTDIR = /opt/ue2
export PREFIX = ue2-
export MAKE = make --no-print-directory 

ifeq ($(OS),Windows_NT)

export PWD=$(CURDIR)
export SUFIX=.exe

else

#export ARCH=-m32
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
CLFAGS += -flto
export LDFLAGS = $(ARCH) -Wl,-no-pie,-flto

all: src
	mkdir -p bin
	$(MAKE) -C cmd -f build.mk
	$(MAKE) -C cmd/as -f build.mk
	$(MAKE) -C cmd/c -f build.mk
	$(MAKE) -C cmd/cpp -f build.mk
# 	$(MAKE) -C cmd/lcpp -f build.mk
#	$(MAKE) -C src -f build.mk all

clean:
	rm -f bin/*
	$(MAKE) -C cmd/cpp -f build.mk clean
#	$(MAKE) -C cmd/c -f build.mk clean
#	$(MAKE) -C cmd/lcpp -f build.mk clean


test-print: all
	mkdir -p tmp
	rm -f tmp/*
	(pushd test && ./tests.sh --detailed; popd)

test: all
	mkdir -p tmp
	rm -f tmp/*
	(pushd test && ./tests.sh; popd)

test-one: all
# ./bin/ue2-ccom test/test1.c -o test1.tok
	mkdir -p tmp
	rm -f tmp/*
	(pushd test && ./tests.sh --detailed align-noop; popd)

#	./bin/ue2-cpp -Itest test/hellorld.s -o test/hellorld.asm
#	./bin/ue2-as test/hellorld.asm -o $(TMP_DIR)/hellorld.out
#	./bin/ue2-as test/def.s -o $(TMP_DIR)/def.out
##	./bin/ue2-size $(TMP_DIR)/*.out
##	./bin/ue2-hdr $(TMP_DIR)/*.out
#	./bin/ue2-ld -o $(TMP_DIR)/test1.out -L $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
#	./bin/ue2-nm -p $(TMP_DIR)/test1.out
##./bin/ue2-ld -s -o $(TMP_DIR)/test1-strip.out $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
##./bin/ue2-ld -x -o $(TMP_DIR)/test1-ext.out $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
#	./bin/ue2-ld -b -o $(TMP_DIR)/test1.bin -L $(TMP_DIR)/hellorld.out $(TMP_DIR)/def.out
	