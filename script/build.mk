# Directorys
BUILD_DIR = build
CLEAN += $(BUILD_DIR)/

TMP_DIR = tmp-src
CLEAN += $(TMP_DIR)/

OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)-$(VERSION)
OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRCS:%.cc=$(OBJ_DIR)/%.o)

BINARY = $(BUILD_DIR)/$(NAME)-$(VERSION)


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
INCLUDES  = $(addprefix -I, $(INC_PATH))

C_SETS   = $($(VERSION)_C_SETS) -MMD -c
LDSETS   = $($(VERSION)_LDSETS)
LDLIBS   = $($(VERSION)_LDLIBS)

CC_STD   = --std=gnu11
CXXSTD   = --std=c++17

CCFLAGS  = $(LDSETS) $(C_SETS) $(INCLUDES) $(CC_STD)
CXXFLAGS = $(LDSETS) $(C_SETS) $(INCLUDES) $(CXXSTD)
LDFLAGS  = $(LDSETS) $(LDLIBS)

-include $(OBJS:%.o=%.d)

$(OBJ_DIR)/%.o: %.c
	@echo '+ CC $<'
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo '+ CXX $<'
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -o $@ $<


# Link rules
$(OBJS): | $(CSRCS) $(CXXSRCS)
$(BINARY): $(OBJS)
	@echo '+ LD $(OBJS)'
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
