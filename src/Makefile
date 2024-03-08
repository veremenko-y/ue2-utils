BIN = $(BIN_DIR)/$(PREFIX)as

APPS = ld nm rel size
BIN = $(APPS:%=$(BIN_DIR)/$(PREFIX)%)
SRC = $(APPS:%=%.c)
OBJ = $(SRC:%.c=%.o)

.PHONY : all clean

all: $(BIN)

clean:
	rm -f $(BIN)
	rm -f *.o

$(BIN_DIR)/$(PREFIX)%: %.o | $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@