# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
CLEAN_DIR += $(OUTPUT_DIR)


# Test rules
$(OUTPUT_DIR)/%.out: %.sy $(BINARY)
	@echo '> test $<'
	@mkdir -p $(dir $@)
	@cat $< | $(BINARY) > $@


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

TESTOUT = $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)
test: $(TESTOUT)
PHONY += test