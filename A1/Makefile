# Compiler settings
CC = g++
CFLAGS = --std=c++11 -Wall

# Output executable name
OUTPUT = inverted_index

# Source files
SRC = inverted_index.cpp pugixml.cpp

# Compile the program
$(OUTPUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SRC)

# Clean up object files and executable
clean:
	rm -f $(OUTPUT)