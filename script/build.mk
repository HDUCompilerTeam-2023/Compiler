# Directorys
BUILD_DIR = build
CLEAN += $(BUILD_DIR)/

TMP_DIR = tmp-src
CLEAN += $(TMP_DIR)/

OBJ_DIR = $(BUILD_DIR)/obj-$(NAME)-$(VERSION)
OBJS = $(CSRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRCS:%.cc=$(OBJ_DIR)/%.o)

TMP_OBJ_DIR = $(OBJ_DIR)/obj-$(NAME)-$(VERSION)_tmp
OBJS += $(TMPCSRCS:$(TMP_DIR)/%.c=$(TMP_OBJ_DIR)/%.o)

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
INC_PATH += include
INCLUDES  = $(addprefix -I, $(INC_PATH)) $(addprefix -I, $(TMP_DIR))

C_SETS   = $($(VERSION)_C_SETS) -MMD -c
LDSETS   = $($(VERSION)_LDSETS)
LDLIBS   = $($(VERSION)_LDLIBS)

CC_STD   = --std=c11
CXXSTD   = --std=c++17

CCFLAGS      = $(LDSETS) $(C_SETS) $(INCLUDES) $(CC_STD) $(NOTMP_C_SETS)
CXXFLAGS     = $(LDSETS) $(C_SETS) $(INCLUDES) $(CXXSTD) $(NOTMP_C_SETS)
CCFLAGS_TMP  = $(LDSETS) $(C_SETS) $(INCLUDES) $(CC_STD)
LDFLAGS      = $(LDSETS) $(LDLIBS)

-include $(OBJS:%.o=%.d)

$(OBJ_DIR)/%.o: %.c
	@echo '+ CC $<'
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS) -o $@ $<

$(OBJ_DIR)/%.o: %.cc
	@echo '+ CXX $<'
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) -o $@ $<

$(TMP_OBJ_DIR)/%.o: $(TMP_DIR)/%.c
	@echo '+ CC_TMP $<'
	@mkdir -p $(dir $@)
	@$(CC) $(CCFLAGS_TMP) -o $@ $<

# Link rules
$(OBJS): | $(CSRCS) $(CXXSRCS) $(TMPCSRCS)
$(BINARY): $(OBJS)
	@echo '+ LD $(OBJS)'
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
