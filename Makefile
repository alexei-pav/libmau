TARGET := libmau.a

CC = musl-gcc
AR = ar
CFLAGS = -g -c -Wall -Werror \
				 -I$(INC_DIR) \
				 -I$(MBEDTLS_INC_DIR) \
				 -I$(CJSON_INC_DIR)

# directories
SRC_DIR := src
BUILD_DIR := build
INC_DIR := include

DEPS_DIR := third_party

MBEDTLS_DIR := $(DEPS_DIR)/mbedtls
MBEDTLS_INC_DIR := $(MBEDTLS_DIR)/include

CJSON_DIR := $(DEPS_DIR)/cjson
CJSON_INC_DIR := $(CJSON_DIR)

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
