# Define compiler and flags
CC = g++
CFLAGS = -g -Wall `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs`

# Define the target executable
TARGET = chip8-emulator

# Define source and header files
SRCS = main.cpp chip8.cpp
HEADERS = chip8.h

# Define object files
OBJS = $(SRCS:.cpp=.o)

# Default build target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	@echo "Linking $@"
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.cpp $(HEADERS)
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
