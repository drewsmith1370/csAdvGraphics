# Top-level Makefile

# List of subdirectories
SUBDIRS = cube lib

# Targets to be built in each subdirectory
TARGET = all

# Default target to build all subdirectories
all: $(SUBDIRS)

# Rule to build each subdirectory
$(SUBDIRS):
	@echo "Building in directory $@..."
	$(MAKE) -C $@ all

# Clean all subdirectories
clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# Run all targets
run:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir run; \
	done

# .PHONY to ensure these targets are always executed
.PHONY: all $(SUBDIRS) clean distclean