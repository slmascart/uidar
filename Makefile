# Inspired by the work of Chase Lambert (https://makefiletutorial.com/#makefile-cookbook)
CC := gcc
CFLAGS := -lwiringPi -lm

TARGET_EXEC := uidar

BIN_DIR := bin/
BUILD_DIR := .build/
SRC_DIRS := src/

# Find all the C files we want to compile
SRCS := $(shell find $(SRC_DIRS) -name '*.c')

# Prepends BUILD_DIR and appends .o to every src file
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -MP

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) -lwiringPi
	mkdir -p $(BIN_DIR)
	cp $@ $(BIN_DIR)/$(TARGET_EXEC)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the error of missing
# Makefiles. Initially, all the .d files will be missing, nd we don't want to those
# errors to show up.
-include $(DEPS)

