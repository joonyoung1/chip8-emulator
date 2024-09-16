# Define compiler and flags
CC = g++
CFLAGS = -g `sdl2-config --cflags` 
LDFLAGS = `sdl2-config --libs`

# Define the target executable
TARGET = chip8-emulator

# Define source files
SRCS = main.cpp chip8.cpp

# Define object files
OBJS = $(SRCS:.cpp=.o)

# Default build target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile source files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule to remove object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
