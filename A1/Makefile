# Compiler settings
CXX = g++
CXXFLAGS = --std=c++17 -Wall

# Output executable name
INVERT_INDEX_TARGET = inverted_index
SEARCH_TARGET = search

# Source files
SRC = invidx_cons.cpp pugixml.cpp tokenizer.cpp utils.cpp
OBJS = $(SRC:.cpp=.o)

SEARCH_SRC = search.cpp tokenizer.cpp utils.cpp pugixml.cpp
SEARCH_OBJS = $(SEARCH_SRC:.cpp=.o)

# Headers
HEADERS = tokenizer.h pugixml.hpp utils.h

# Default target to build everything
my_index: $(INVERT_INDEX_TARGET)

my_search: $(SEARCH_TARGET)

# Link the object files to produce the final executable
$(INVERT_INDEX_TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(INVERT_INDEX_TARGET)

$(SEARCH_TARGET): $(SEARCH_OBJS)
	$(CXX) $(SEARCH_OBJS) -o $(SEARCH_TARGET)

# Rule to produce the object files from the source files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@


# Clean up object files and executable
clean:
	rm -f $(OBJS) $(INVERT_INDEX_TARGET) $(SEARCH_TARGET) $(SEARCH_OBJS)