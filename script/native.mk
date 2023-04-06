# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
OUTPUT_FAILURE = $(OUTPUT_DIR)/failure
CLEAN += $(OUTPUT_DIR)/


# Test rules
$(OUTPUT_DIR)/%.out: %.sy $(BINARY) ALWAYS
	@echo '> test $<'
	@mkdir -p $(dir $@)
	@$(BINARY) $< > $@ 2>&1 || echo $@ >> $(OUTPUT_FAILURE)


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

gdb: $(BINARY)
	@echo '> gdb $^'
	@gdb -q $(BINARY)
PHONY += run

test: $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)
	@cat $(OUTPUT_FAILURE) 2> /dev/null && rm $(OUTPUT_FAILURE) && exit 1 || echo no error
PHONY += test
