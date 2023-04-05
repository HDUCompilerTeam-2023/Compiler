# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
OUTPUT_FAILURE = $(OUTPUT_DIR)/failure
CLEAN += $(OUTPUT_DIR)/


# Test rules
$(OUTPUT_FAILURE): $(BINARY)
	@mkdir -p $(dir $@)
	@rm -f $@

$(OUTPUT_DIR)/%.out: %.sy $(BINARY)
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

test: $(OUTPUT_FAILURE) $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)
	@cat $(OUTPUT_FAILURE) 2> /dev/null && exit 1 || echo no error
PHONY += test