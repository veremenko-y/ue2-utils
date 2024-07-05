include $(PWD)/common.mk
CFLAGS += -I$(PWD)/include

.PHONY: all test

OBJ_FILES=as0 as1 as2 as
OUT=$(BIN_DIR)/$(call out_name,as)


all: $(OUT)

# test: all
# 	./ue2-as hellorld.s -o hellorld.out && \
# 	./ue2-as def.s -o def.out


$(OUT): $(OBJ_FILES:%=$(BIN_DIR)/%.o)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR)/%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@
