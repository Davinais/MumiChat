CC = gcc
CFLAGS = 

BIN_DIR = bin
BUILD_DIR = obj
SRC_DIR = src

SOURCES := $(wildcard $(SRC_DIR)/*.c)
INCLUDES := ./$(SRC_DIR)

all: $(BIN_DIR)/server $(BIN_DIR)/client

$(BIN_DIR)/server: $(BUILD_DIR)/server.o $(BUILD_DIR)/list.o
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(BUILD_DIR)/server.o $(BUILD_DIR)/list.o

$(BIN_DIR)/client: $(BUILD_DIR)/client.o
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ -pthread $(BUILD_DIR)/client.o

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/packet.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -I$(INCLUDES) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/* $(BIN_DIR)/*