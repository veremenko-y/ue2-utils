export CC := gcc
export ARCH= -m32
export CFLAGS =

export BIN_DIR = $(PWD)/bin
export TMP_DIR = $(PWD)/tmp
export DESTDIR = /opt/ue2
export PREFIX = ue2-

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
