# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
FAILURE = $(OUTPUT_DIR)/failure
CLEAN += $(OUTPUT_DIR)/


# Test rules
$(OUTPUT_DIR)/%.out: %.sy $(BINARY)
	@echo '> test $<'
	@mkdir -p $(dir $@)
	-@cat $< | $(BINARY) > $@ 2> $@.err


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

test: $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)
PHONY += test