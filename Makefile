# Info
NAME ?= RelopLexer


# Compiler
LEX ?= flex
CC ?= clang


# Source
CSRC_DIR ?= src

LEXSRC ?= grammar/Relop.lex
CSRCS = $(shell find $(CSRC_DIR) -name *.c)
CXXSRCS = $(shell find $(CSRC_DIR) -name *.cc)


# Rules
include script/native.mk


# Settings
.PHONY: $(PHONY)

## do not remove secondary file
.SECONDARY:
