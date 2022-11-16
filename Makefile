# Info
NAME ?= SysYLexer


# Compiler
YACC ?= bison
LEX ?= flex
CC ?= clang


# Debug args only
YACCFLAG = -r solved
LEXFLAG =

INC_PATH =
CFLAGS = -DDEBUG

LDFLAGS =
LDLIBS =


# Source
CSRC_DIR ?= src
TEST_DIR ?= test

YACCSRC ?= grammar/SysY.y
LEXSRC ?= grammar/SysY.l
CSRCS = $(shell find $(CSRC_DIR) -name *.c)
CXXSRCS = $(shell find $(CSRC_DIR) -name *.cc)

TESTSRC = $(shell find $(TEST_DIR) -name *.sy)


# Rules
include script/native.mk


# Phony rules
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
