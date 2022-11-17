# Settings
.DEFAULT_GOAL = app


# Directorys
BUILD_DIR = build
CLEAN += $(BUILD_DIR)/

TMP_DIR = tmp-src
CLEAN += $(TMP_DIR)/

OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)$(SO)
OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRC:%.cc=$(OBJ_DIR)/%.o)

BINARY = $(BUILD_DIR)/$(NAME)


# Yacc rules
YACCFLAGS += -d

$(TMP_DIR)/%.tab.h $(TMP_DIR)/%.tab.c: %.y
	@echo + YACC $<
	@mkdir -p $(dir $@)
	@$(YACC) $(YACCFLAGS) -o $(@:%.h=%.c) $<

CSRCS += $(YACCSRC:%.y=$(TMP_DIR)/%.tab.c)


# Lex rules
LEXFLAGS +=

$(TMP_DIR)/%.yy.c: %.l
	@echo + LEX $<
	@mkdir -p $(dir $@)
	@$(LEX) $(LEXFLAGS) -o $@ $<

CSRCS += $(LEXSRC:%.l=$(TMP_DIR)/%.yy.c)


# Compile rules
ifeq ($(CC),clang)
CXX = clang++
else
CXX = g++
endif
LD = $(CXX)

INC_PATH += include
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS  += -Wall -Werror -O2 $(INCLUDES)

LDFLAGS += -O2
LDLIBS += -lfl

$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) -c -o $@ $<


# Link rules
$(BINARY): $(OBJS)
	@echo + LD $^
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) $(LDLIBS) -o $@ $^


# Phony rules
app: $(BINARY)
PHONY += app
