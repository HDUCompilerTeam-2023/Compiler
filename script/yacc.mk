# Yacc rules
YACCFLAGS += -d

$(TMP_DIR)/%.tab.h $(TMP_DIR)/%.tab.c: %.y
	@echo '+ YACC $<'
	@mkdir -p $(dir $@)
	@$(YACC) $(YACCFLAGS) -o $(@:%.h=%.c) $<

CSRCS += $(YACCSRC:%.y=$(TMP_DIR)/%.tab.c)
