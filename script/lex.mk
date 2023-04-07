# Lex rules
LEXFLAGS +=

$(TMP_DIR)/%.yy.h $(TMP_DIR)/%.yy.c: %.l
	@echo '+ LEX $<'
	@mkdir -p $(dir $@)
	@$(LEX) $(LEXFLAGS) --header-file=$(@:%.c=%.h) -o $@ $<

CSRCS += $(LEXSRC:%.l=$(TMP_DIR)/%.yy.c)
