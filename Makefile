# Info
NAME ?= SysYParser
VERSION ?= debug


# Compiler
YACC = bison
LEX = flex
CC = clang


# Debug args only
YACCFLAGS = -r solved
LEXFLAGS  =

INC_PATH =

debug_C_SETS   = -g -Wall -Werror -DDEBUG
debug_LDSETS   = -fsanitize=address -fsanitize=leak
debug_LDLIBS   =

release_C_SETS = -O2
release_LDSETS =
release_LDLIBS =


# Source
CSRC_DIR = src
TEST_DIR = test

YACCSRC = grammar/SysY.y
LEXSRC = grammar/SysY.l
CSRCS = $(shell find $(CSRC_DIR) -name "*.c")
CXXSRCS = $(shell find $(CSRC_DIR) -name "*.cc")

TESTSRC = $(shell find $(TEST_DIR) -name "*.sy")

BUILD_SCIRPT = script/yacc.mk script/lex.mk


# Rules
include script/native.mk


# Phony rules
clean:
	@echo '- CLEAN $(CLEAN)'
	@rm -rf $(CLEAN)
PHONY += clean

help:
	@echo ': commands'
	@echo '  $(PHONY)'
PHONY += help


# Sanity check
ifneq ($(VERSION),debug)
  ifneq ($(VERSION),release)
$(error $$(VERSION) ($(VERSION)) is not correct, need 'debug' or 'release')
  endif
endif


# Settings
.PHONY: $(PHONY)

## Always tag
ALWAYS:
.PHONY: ALWAYS

## do not remove secondary file
.SECONDARY:
