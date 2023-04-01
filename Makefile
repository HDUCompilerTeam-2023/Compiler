# Info
NAME ?= SysYParser


# Compiler
YACC = bison
LEX = flex
CC = clang


# Debug args only
YACCFLAGS = -r solved
LEXFLAGS  =

INC_PATH =
C_FLAGS  = -DDEBUG
LDFLAGS  = -fsanitize=address -fsanitize=leak
LDLIBS   =


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
compinfo:
	@echo ': Compiler Info'
	@echo '  LEX  : $(LEX)'
	@echo '  YACC : $(YACC)'
	@echo '  CC   : $(CC)'
	@echo '  CXX  : $(CXX)'
	@echo '  LEXFLAGS  : $(LEXFLAGS)'
	@echo '  YACCFLAGS : $(YACCFLAGS)'
	@echo '  INCLUDES  : $(INCLUDES)'
	@echo '  C_FLAGS   : $(C_FLAGS)'
	@echo '  LDFLAGS   : $(LDFLAGS)'
	@echo '  LDLIBS    : $(LDLIBS)'
PHONY += compinfo

clean:
	@echo - CLEAN $(CLEAN)
	@rm -rf $(CLEAN)
PHONY += clean

help:
	@echo : commands
	@echo '  $(PHONY)'
PHONY += help



# Settings
.PHONY: $(PHONY)

## do not remove secondary file
.SECONDARY:
