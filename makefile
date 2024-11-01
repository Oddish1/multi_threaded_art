# Define the compiler
CXX = g++

# Define compiler flags
CXXFLAGS = -Wall -g

# Define the path to the SFML include directory
SFML_INCLUDE_DIR = /usr/include/SFML

# Define the path to the SFML libraries
SFML_LIB_DIR = ./libs

# Define libraries for linking
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Define target executable and its directory
TARGET = bin/multi_threaded_artwork

# Define source files and object files
SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Ensure the bin and obj directories are created
$(shell mkdir -p bin obj)

# Target
all: $(TARGET)

# Link the target with object files
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) -L$(SFML_LIB_DIR) $(LIBS)

# Compile the source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(SFML_INCLUDE_DIR) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)
	rm -f multi_threaded_art.png