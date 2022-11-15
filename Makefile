# Info
NAME ?= SysYLexer


# Compiler
LEX ?= flex
CC ?= clang


# Source
CSRC_DIR ?= src
TEST_DIR ?= test

LEXSRC ?= grammar/SysY.lex
CSRCS = $(shell find $(CSRC_DIR) -name *.c)
CXXSRCS = $(shell find $(CSRC_DIR) -name *.cc)

TESTSRC = $(shell find $(TEST_DIR) -name *.sy)

# Rules
include script/native.mk

clean:
	@echo - CLEAN $(CLEAN_DIR)
	@rm -rf $(CLEAN_DIR)
PHONY += clean

help:
	@echo : commands
	@echo $(PHONY)

# Settings
.PHONY: $(PHONY)

## do not remove secondary file
.SECONDARY:
