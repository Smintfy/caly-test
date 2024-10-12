# CMake for MacOS

# Compiler
CC = clang

# Flags
CFLAGS = -Wall -Wextra

# Lib path
LIBDIR = lib

# Libraries
LIBS = -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
RAYLIB = $(LIBDIR)/libraylib.a

# Source files
SRCS = $(wildcard *.c)

# Object files
OBJS = $(SRCS:.c=.o)

EXEC = caly

# Default target
all: $(EXEC)

# Linking
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -L$(LIBDIR) $(LIBS) $(RAYLIB) -o $@

# Compiling
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJS) $(EXEC)

# Run the program
run: $(EXEC)
	./$(EXEC)

# Build and run
build: $(EXEC) run

.PHONY: all clean run build
