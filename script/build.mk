# Settings
.DEFAULT_GOAL = app


# Directorys
BUILD_DIR = build
CLEAN += $(BUILD_DIR)/

TMP_DIR = tmp-src
CLEAN += $(TMP_DIR)/

OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)$(SO)
OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRCS:%.cc=$(OBJ_DIR)/%.o)

BINARY = $(BUILD_DIR)/$(NAME)


# Compilers
ifeq ($(CC),clang)
CXX = clang++
else
CXX = g++
endif
LD = $(CXX)


# SRC from other source
## Just can change $(CSRCS) and $(CXXSRCS)
include $(BUILD_SCIRPT)


# Compile rules
INC_PATH += include $(TMP_DIR)
INCLUDES = $(addprefix -I, $(INC_PATH))
CFLAGS  += -Wall -Werror -O2 -MMD $(INCLUDES)
CXXFLAGS += $(CFLAGS)

LDFLAGS += -O2
LDLIBS += -lfl

-include $(OBJS:%.o=%.d)

$(OBJ_DIR)/%.o: %.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<


# Link rules
$(BINARY): $(CSRCS) $(CXXSRCS) $(OBJS)
	@echo + LD $(OBJS)
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) $(LDLIBS) -o $@ $(OBJS)


# Phony rules
app: $(BINARY)
PHONY += app
