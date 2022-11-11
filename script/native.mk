# Build rules
include script/build.mk

# Phony rules
run: $(BINARY)
	@echo '> run $^'
	@$(BINARY)
PHONY += run

