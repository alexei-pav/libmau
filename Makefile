TARGET := libmau.a

CC = musl-gcc
AR = ar
CFLAGS = -g -c -Wall -Werror \
				 -I$(INC_DIR)

# directories
SRC_DIR := src
BUILD_DIR := build
INC_DIR := include

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf build/
	rm -f $(TARGET)
	make clean -C $(CJSON_DIR)
	make clean -C $(MBEDTLS_DIR)
