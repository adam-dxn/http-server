CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -Iserver/include -Iserver/src

# SSL librarys
LDFLAGS = -lssl -lcrypto

# The test program uses curl to send requests to the server.
TEST_LDFLAGS = -lcurl

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
TEST_OBJ_DIR = $(BUILD_DIR)/test_obj

SRC = $(wildcard server/src/*.c)
OBJ = $(patsubst server/src/%.c,$(OBJ_DIR)/%.o,$(SRC))

TEST_SRC = curl_client.c
TEST_TARGET = curl_client

TARGET = http_server

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

tests: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC)
	$(CC) $(CFLAGS) $< -o $@ $(TEST_LDFLAGS)

$(OBJ_DIR)/%.o: server/src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)
