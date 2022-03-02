# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------


#directories
SRC_DIR := src
HDR_DIR := include
OBJ_DIR := obj


#files
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp -o -name *.cxx -o -name *.c++ -o -name *.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(patsubst %,%.o,$(SRC_FILES)))
BIN := $(BIN_DIR)/generator



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

build: clean all

all: $(BIN)

$(BIN): $(OBJ_FILES)
	$(CXX) $(FLAGS) $^ -o $@ $(LINKER_FLAGS)

#generate each object file according to the corresponding source file
#create directories as needed
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) -c $< -o $@





#'clean' and 'init' don't represent actual file generating recipes
.PHONY: clean init

clean:
	-find $(OBJ_DIR)/* | grep -v $(OBJ_DIR)/*.md | xargs rm

init:
	-mkdir $(SRC_DIR)
	-mkdir $(HDR_DIR)
	-mkdir $(OBJ_DIR)
	-mkdir $(BIN_DIR)