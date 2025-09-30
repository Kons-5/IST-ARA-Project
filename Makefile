TARGET_BASE := routing
CC := gcc
CSTD := -std=gnu99
WARN := -Wall -Werror -Wpedantic -Wconversion
CFLAGS_COMMON := $(CSTD) $(WARN)
CFLAGS_REL := $(CFLAGS_COMMON) -O3
CFLAGS_DBG := $(CFLAGS_COMMON) -ggdb
LDFLAGS :=

BIN_DIR := bin
OBJ_ROOT := obj
OBJ_REL := $(OBJ_ROOT)/release
OBJ_DBG := $(OBJ_ROOT)/debug

HDR_COMMON := $(wildcard include/*.h)
HDR_SEQ := $(wildcard include/sequential/*.h)
HDR_DIST := $(wildcard include/distributed/*.h)

SRCS_SEQ := $(wildcard src/sequential/*.c)
SRCS_DIST := $(wildcard src/distributed/*.c)

OBJS_SEQ_REL := $(patsubst src/sequential/%.c,$(OBJ_REL)/sequential/%.o,$(SRCS_SEQ))
OBJS_DIST_REL := $(patsubst src/distributed/%.c,$(OBJ_REL)/distributed/%.o,$(SRCS_DIST))
OBJS_SEQ_DBG := $(patsubst src/sequential/%.c,$(OBJ_DBG)/sequential/%.o,$(SRCS_SEQ))
OBJS_DIST_DBG := $(patsubst src/distributed/%.c,$(OBJ_DBG)/distributed/%.o,$(SRCS_DIST))

TARGET_SEQ_REL := $(BIN_DIR)/$(TARGET_BASE)-seq
TARGET_DIST_REL := $(BIN_DIR)/$(TARGET_BASE)-dist
TARGET_SEQ_DBG := $(BIN_DIR)/$(TARGET_BASE)-seq-debug
TARGET_DIST_DBG := $(BIN_DIR)/$(TARGET_BASE)-dist-debug

.DEFAULT_GOAL := all

.PHONY: all debug dist seq debug-dist debug-seq help clean
all: dist seq ## build both sequential and distributed (release)
debug: debug-dist debug-seq ## build both sequential and distributed (debug)

help: ## list all available make targets
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
	 | sort \
	 | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-24s\033[0m %s\n", $$1, $$2}'

$(OBJ_REL)/distributed/%.o: src/distributed/%.c $(HDR_COMMON) $(HDR_DIST) Makefile | $(OBJ_REL)/distributed
	$(CC) -c $(CFLAGS_REL) -Iinclude -Iinclude/distributed $< -o $@

$(OBJ_REL)/sequential/%.o: src/sequential/%.c $(HDR_COMMON) $(HDR_SEQ) Makefile | $(OBJ_REL)/sequential
	$(CC) -c $(CFLAGS_REL) -Iinclude -Iinclude/sequential $< -o $@

$(OBJ_DBG)/distributed/%.o: src/distributed/%.c $(HDR_COMMON) $(HDR_DIST) Makefile | $(OBJ_DBG)/distributed
	$(CC) -c $(CFLAGS_DBG) -Iinclude -Iinclude/distributed $< -o $@

$(OBJ_DBG)/sequential/%.o: src/sequential/%.c $(HDR_COMMON) $(HDR_SEQ) Makefile | $(OBJ_DBG)/sequential
	$(CC) -c $(CFLAGS_DBG) -Iinclude -Iinclude/sequential $< -o $@

$(BIN_DIR):
	@mkdir -p $@

$(OBJ_REL)/distributed $(OBJ_REL)/sequential $(OBJ_DBG)/distributed $(OBJ_DBG)/sequential:
	@mkdir -p $@

clean: ## remove all build artifacts
	@rm -rf $(OBJ_ROOT) $(BIN_DIR) vgcore* core*

ifneq ($(strip $(SRCS_DIST)),)
dist: $(TARGET_DIST_REL) ## build only distributed (release)
debug-dist: $(TARGET_DIST_DBG) ## build only distributed (debug)

$(TARGET_DIST_REL): $(OBJS_DIST_REL) | $(BIN_DIR)
	$(CC) $(CFLAGS_REL) $(OBJS_DIST_REL) $(LDFLAGS) -o $@

$(TARGET_DIST_DBG): $(OBJS_DIST_DBG) | $(BIN_DIR)
	$(CC) $(CFLAGS_DBG) $(OBJS_DIST_DBG) $(LDFLAGS) -o $@
else
dist debug-dist:
	@echo "(skip) distributed: no sources in 'src/distributed'"
endif

ifneq ($(strip $(SRCS_SEQ)),)
seq: $(TARGET_SEQ_REL) ## build only sequential (release)
debug-seq: $(TARGET_SEQ_DBG) ## build only sequential (debug)

$(TARGET_SEQ_REL): $(OBJS_SEQ_REL) | $(BIN_DIR)
	$(CC) $(CFLAGS_REL) $(OBJS_SEQ_REL) $(LDFLAGS) -o $@

$(TARGET_SEQ_DBG): $(OBJS_SEQ_DBG) | $(BIN_DIR)
	$(CC) $(CFLAGS_DBG) $(OBJS_SEQ_DBG) $(LDFLAGS) -o $@
else
seq debug-seq:
	@echo "(skip) sequential: no sources in 'src/sequential'"
endif
