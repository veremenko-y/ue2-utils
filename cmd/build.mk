include $(PWD)/common.mk
CFLAGS += -I$(PWD)/include

.PHONY: all test

PROGRAMS=nm size ld dis hdr ar

all: $(PROGRAMS:%=$(BIN_DIR)/$(call out_name,%))

$(BIN_DIR)/$(call out_name,%): %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@

# clean:
# 	rm -f $(PROGRAMS) $(PROGRAMS:%=ue2-%)
# 	rm -f *.out
