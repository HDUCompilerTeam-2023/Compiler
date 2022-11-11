# Settings
.DEFAULT_GOAL = app


# Directorys 
BUILD_DIR = build
TMP_DIR = tmp
CLEAN_DIR += $(BUILD_DIR) $(TMP_DIR)

OBJ_DIR  = $(BUILD_DIR)/obj-$(NAME)$(SO)
BINARY = $(BUILD_DIR)/$(NAME)


# Lex rules
LEXFLAG = -i -I

$(TMP_DIR)/%.yy.c: %.lex
	@echo + LEX $<
	@mkdir -p $(dir $@)
	@$(LEX) $(LEXFLAG) -o $@ $<


# Compile rules
ifeq ($(CC),clang)
CXX = clang++
else
CXX = g++
endif
LD = $(CXX)

INC_PATH += include
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS  += -O2 -MMD $(INCLUDES)
CFLAGS_WCHECK = -Wall -Werror

LDFLAGS += -O2
LDLIBS += -lfl

$(OBJ_DIR)/%.o: $(TMP_DIR)/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(TMP_DIR)/%.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_WCHECK) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS_WCHECK) $(CFLAGS) -c -o $@ $<


# Link rules
OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRC:%.cc=$(OBJ_DIR)/%.o) $(LEXSRC:%.lex=$(OBJ_DIR)/%.yy.o)

$(BINARY): $(OBJS)
	@echo + LD $^
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) $(LDLIBS) -o $@ $^


# Phony rules
app: $(BINARY)
PHONY += app

clean:
	@echo - CLEAN $(CLEAN_DIR)
	@rm -rf $(CLEAN_DIR)
PHONY += clean

