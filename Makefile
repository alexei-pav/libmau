TARGET := libmau.a

CC = musl-gcc
AR = ar
CFLAGS = -g -c -Wall -Werror \
				 -I$(INC_DIR) \
				 -I$(CJSON_INC_DIR) \
				 -I$(MBEDTLS_INC_DIR)

# directories
SRC_DIR := src
BUILD_DIR := build
INC_DIR := include

DEPS_DIR := third_party

CJSON_DIR := $(DEPS_DIR)/cjson
CJSON_INC_DIR := $(CJSON_DIR)
CJSON_LIB := $(CJSON_DIR)/libcjson.a

MBEDTLS_DIR := $(DEPS_DIR)/mbedtls
MBEDTLS_INC_DIR := $(MBEDTLS_DIR)/include
MBEDTLS_LIB := $(MBEDTLS_DIR)/library/libmbedtls.a \
							 $(MBEDTLS_DIR)/library/libmbedx509.a \
							 $(MBEDTLS_DIR)/library/libmbedcrypto.a

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))

all: $(TARGET)

$(TARGET): $(OBJ) $(CJSON_LIB) $(MBEDTLS_LIB)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(CJSON_LIB):
	make -C $(CJSON_DIR)

$(MBEDTLS_LIB):
	make -C $(MBEDTLS_DIR)

clean:
	rm -rf build/
	rm -f $(TARGET)
	make clean -C $(CJSON_DIR)
	make clean -C $(MBEDTLS_DIR)
