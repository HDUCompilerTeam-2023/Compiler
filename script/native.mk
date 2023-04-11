# Build rules
include script/build.mk


# Directorys
OUTPUT_DIR = output
OUTPUT_FAILURE = $(OUTPUT_DIR)/failure
CLEAN += $(OUTPUT_DIR)/


# Test rules
$(OUTPUT_DIR)/%.out: %.sy $(BINARY) ALWAYS
	@echo '  * $<'
	@mkdir -p $(dir $@)
	@echo '=====' >  $@
	@cat $<       >> $@
	@echo ''      >> $@
	@echo '=====' >> $@
	@$(BINARY) $< >> $@ 2>&1 || echo '  x $@' >> $(OUTPUT_FAILURE)


# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

gdb: $(BINARY)
	@echo '> gdb $^'
	@gdb -q $(BINARY)
PHONY += gdb

test: $(TESTSRC:%.sy=$(OUTPUT_DIR)/%.out)
	@test -f $(OUTPUT_FAILURE) \
		&& echo '< Error' \
		&& cat $(OUTPUT_FAILURE) \
		&& rm $(OUTPUT_FAILURE) \
		&& exit 1 \
	|| echo '< No error'
PHONY += test
