# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------



#directories
SRC_DIR 		:= src
INC_DIR 		:= include
OBJ_DIR 		:= obj



#files
SRC_FILES 		:= $(shell find $(SRC_DIR) -name *.cpp -o -name *.cxx -o -name *.c++ -o -name *.cc)
OBJ_FILES 		:= $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(SRC_FILES))
BIN 			:= $(BIN_DIR)/generator



#compiler flags
CXXFLAGS		+= -I$(INC_DIR) -I$(UTILS_DIR)/include

#linker flags
LDFLAGS			:= -L$(UTILS_DIR)/lib

#linker libraries
LDLIBS			:= -lutils



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: $(BIN)

build: clean all

$(BIN): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

#generate each object file according to the corresponding source file
#create directories as needed
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@



#'clean' doesn't represent an actual file generating recipe
.PHONY: clean

clean:
	-rm -rf $(OBJ_DIR)