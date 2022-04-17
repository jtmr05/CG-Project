# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------



#directories
SRC_DIR 		:= src
INC_DIR 		:= include
OBJ_DIR 		:= obj
LIB_DIR 		:= lib



#files
SRC_FILES 		:= $(shell find $(SRC_DIR) -name *.cpp -o -name *.cxx -o -name *.c++ -o -name *.cc)
OBJ_FILES 		:= $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%.o,$(SRC_FILES))
LIB_FILE 		:= $(LIB_DIR)/libtinyxml.a



#compiler flags
CXXFLAGS		   := -Wall -I$(INC_DIR) -O3 -Wno-unknown-pragmas -Wno-format -std=c++17



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: $(LIB_FILE)

build: clean all

#generate each object file according to the corresponding source file
#create directories as needed
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(LIB_FILE): $(OBJ_FILES)
	@mkdir -p $(@D)
	$(AR) $@ $^


#'clean' doesn't represent an actual file generating recipe
.PHONY: clean

clean:
	-rm -rf $(OBJ_DIR)
	-rm -rf $(LIB_DIR)