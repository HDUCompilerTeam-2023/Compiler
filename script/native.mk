# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
CLEAN_DIR += $(OUTPUT_DIR)

FAILURE = $(OUTPUT_DIR)/failure


# Test rules
$(OUTPUT_DIR)/%.out: %.sy $(BINARY)
	@echo '> test $<'
	@mkdir -p $(dir $@)
	@cat $< | $(BINARY) > $@ 2> /dev/null || echo $< >> $(FAILURE)

$(FAILURE): $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

test: $(FAILURE)
	@echo - FAIL
	@cat $(FAILURE)
PHONY += test