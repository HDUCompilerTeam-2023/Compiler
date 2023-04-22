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

-include script/build.local


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
build: pre-build $(BINARY)
	@  echo ': Compiler arguments' \
	&& echo '  LEX  : $(LEX) $(LEXFLAGS)' \
	&& echo '  YACC : $(YACC) $(YACCFLAGS)' \
	&& echo '  CC   : $(CC) $(CCFLAGS)' \
	&& echo '  CXX  : $(CXX) $(CXXFLAGS)' \
	&& echo '  LD   : $(LD) $(LDFLAGS)'
PHONY += build

rebuild: clean build
PHONY += rebuild

clean:
	@echo '- CLEAN $(CLEAN)'
	@rm -rf $(CLEAN)
PHONY += clean

help:
	@  echo ': commands' \
	&& echo '  $(PHONY)'
PHONY += help


# Inner phony target
pre-build:
	@  echo ': Compiler version' \
	&& echo '  $(shell $(LEX) --version | grep $(LEX))' \
	&& echo '  $(shell $(YACC) --version | grep $(YACC))' \
	&& echo '  $(shell $(CC) --version | grep $(CC))'
.PHONY : pre-build


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

## Default target
.DEFAULT_GOAL = build
