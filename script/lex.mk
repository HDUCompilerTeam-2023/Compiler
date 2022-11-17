# Lex rules
LEXFLAGS +=

$(TMP_DIR)/%.yy.c: %.l
	@echo + LEX $<
	@mkdir -p $(dir $@)
	@$(LEX) $(LEXFLAGS) -o $@ $<

CSRCS += $(LEXSRC:%.l=$(TMP_DIR)/%.yy.c)
