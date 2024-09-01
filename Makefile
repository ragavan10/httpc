# Variables
CC = gcc
TEST_CFLAGS = -Wall -g -O -Iinclude
CFLAGS = -Wall -g -fPIC -Iinclude
TEST_LDFLAGS = -lcjson -lpthread -lm -lrt -lcheck -lsubunit 
LDFLAGS = -shared 
TARGET = libhttpc.so
SRCDIR = src
TEST_SRCDIR = src/tests
BUILDDIR = build
TEST_BUILDDIR = test_build

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.c)
# List of test source files
TEST_SRCS = $(wildcard $(TEST_SRCDIR)/*.c) $(SRCS)
# Automatically derive the list of object files from the source files
OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))
# TEST_OBJS = $(patsubst $(TEST_SRCDIR)/%.c, $(TEST_BUILDDIR)/%.o, $(TEST_SRCS))
# Default rule
all: $(TARGET)

# Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

test:
	$(CC) $(TEST_CFLAGS) $(TEST_SRCS) $(TEST_LDFLAGS) -o $(TEST_BUILDDIR)/test
	cd $(TEST_BUILDDIR) && ./test


# Rule to compile .c files into .o files
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)  # Create the build directory if it doesn't exist
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile .c files into .o files
# $(TEST_BUILDDIR)/%.o: $(TEST_SRCDIR)/%.c
# 	@mkdir -p $(TEST_BUILDDIR)  # Create the build directory if it doesn't exist
# 	$(CC) $(TEST_CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	rm -f $(BUILDDIR)/*.o $(TARGET)
	rm -f $(TEST_BUILDDIR)/*.o test

# Phony targets
.PHONY: all clean


