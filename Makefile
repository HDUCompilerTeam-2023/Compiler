# Info
NAME ?= SysYParser


# Compiler
YACC = bison
LEX = flex
CC = clang


# Debug args only
YACCFLAGS = -r solved
LEXFLAGS =

INC_PATH =
CFLAGS = -DDEBUG
CXXFLAGS =

LDFLAGS =
LDLIBS =


# Source
CSRC_DIR = src
TEST_DIR = test

YACCSRC = grammar/SysY.y
LEXSRC = grammar/SysY.l
CSRCS = $(shell find $(CSRC_DIR) -name *.c)
CXXSRCS = $(shell find $(CSRC_DIR) -name *.cc)

TESTSRC = $(shell find $(TEST_DIR) -name *.sy)


# Rules
include script/native.mk


# Phony rules
clean:
	@echo - CLEAN $(CLEAN)
	@rm -rf $(CLEAN)
PHONY += clean

help:
	@echo : commands
	@echo $(PHONY)
PHONY += help


# Settings
.PHONY: $(PHONY)

## do not remove secondary file
.SECONDARY:
