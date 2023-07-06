# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
OUTPUT_FAILURE = $(OUTPUT_DIR)/failure
CLEAN += $(OUTPUT_DIR)/

TEST_ASM_SRC = $(shell find $(TEST_DIR) -name "*.s")
TEST_EXE_SRC = $(shell find $(TEST_DIR) -name "*.exe")

ARM_ASSEMBLER = arm-linux-gnueabihf-gcc -static

# Test rules
$(OUTPUT_DIR)/%.compiler_out: %.sy $(BINARY) ALWAYS
	@echo '  * $<'
	@mkdir -p $(dir $@)
	@echo '=====' >  $@
	@cat $<       >> $@
	@echo ''      >> $@
	@echo '=====' >> $@
	@$(BINARY) $< -o $(<:%.sy=%.s) >> $@ 2>&1 || echo '  x $@' >> $(OUTPUT_FAILURE)

LINK_DIR = lib
LINK_FILE = sysy
$(OUTPUT_DIR)/%.asm_out: %.s ALWAYS
	@echo '  * $<'
	@mkdir -p $(dir -p $(dir $@))
	@echo '=====' > $@
	@cat $< >> $@
	@echo ''      >> $@
	@echo '=====' >> $@
	@$(ARM_ASSEMBLER) $< -o $(basename $<).exe -L$(LINK_DIR) -l$(LINK_FILE) >> $@ 2>&1 || echo '  x $@' >> $(OUTPUT_FAILURE)

$(OUTPUT_DIR)/%.run_out: %.exe ALWAYS
		@echo '  * $<'
		@./test_exe.sh $< > $@ 2>&1 || echo '  x $@' >> $(OUTPUT_FAILURE)


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

gdb: $(BINARY)
	@echo '> gdb $^'
	@gdb -q $(BINARY)
PHONY += gdb

test: $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.compiler_out) 
	@echo 'create asm' 
	@test -f $(OUTPUT_FAILURE) \
		&& echo '< Error' \
		&& cat $(OUTPUT_FAILURE) \
		&& rm $(OUTPUT_FAILURE) \
		&& exit 1 \
	|| echo '< No error'
PHONY += test

test_asm: $(TEST_ASM_SRC:%.s=$(OUTPUT_DIR)/%.asm_out) 
	@echo '  test asm'
	@test -f $(OUTPUT_FAILURE) \
		&& echo '< Error' \
		&& cat $(OUTPUT_FAILURE) \
		&& rm $(OUTPUT_FAILURE) \
		&& exit 1 \
	|| echo '< No error'
PHONY += test_asm

test_exe: $(TEST_EXE_SRC:%.exe=$(OUTPUT_DIR)/%.run_out)
	@echo '  test exe'
	@test -f $(OUTPUT_FAILURE) \
		&& echo '< Error' \
		&& cat $(OUTPUT_FAILURE) \
		&& rm $(OUTPUT_FAILURE) \
		&& exit 1 \
	|| echo '< No error'
PHONY += test_exe

test_output: 
	@echo '  test output'
	@./test_output.sh $(TEST_DIR)
PHONY += test_output
