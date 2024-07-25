include $(PWD)/common.mk
CFLAGS += -I$(PWD)/include
LDLIBS += -lreadline
# CFLAGS +=  -fsanitize=address

.PHONY: all test

PROGRAMS=nm size ld dis hdr ar mkfs fsc

all: $(PROGRAMS:%=$(BIN_DIR)/$(call out_name,%)) $(BIN_DIR)/$(call out_name,mkimg)

$(BIN_DIR)/$(call out_name,%): %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LDLIBS) -o $@


$(BIN_DIR)/$(call out_name,mkimg): mkimg.c
	$(CC) -Og -g $(LDFLAGS) $< $(LDLIBS) -o $@


# clean:
# 	rm -f $(PROGRAMS) $(PROGRAMS:%=ue2-%)
# 	rm -f *.out
