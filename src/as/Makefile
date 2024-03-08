BIN = $(BIN_DIR)/$(PREFIX)as
SRC = as.c ue2_def.c
OBJ = $(SRC:%.c=%.o)

.PHONY : all clean

all: $(BIN)

clean:
	rm -f $(BIN)
	rm -f *.o

$(BIN): $(OBJ) | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@