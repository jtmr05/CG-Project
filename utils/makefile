# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------


#directories
SRC_DIR := src
HDR_DIR := include
OBJ_DIR := obj
LIB_DIR := lib


#files
SRC_FILES := $(shell find $(SRC_DIR) -name *.cpp -o -name *.cxx -o -name *.c++ -o -name *.cc)
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(patsubst %,%.o,$(SRC_FILES)))
UTILS_LIB := $(LIB_DIR)/libutils.a

#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

build: clean all

all: $(UTILS_LIB)


#generate each object file according to the corresponding source file
#create directories as needed
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%
	@mkdir -p $(@D)
	$(CXX) $(FLAGS) -c $< -o $@

$(UTILS_LIB): $(OBJ_FILES)
	-find $(LIB_DIR)/* | grep $(LIB_DIR)/*.a | xargs rm
	ar -rcs $@ $^



#'clean' and 'init' don't represent actual file generating recipes
.PHONY: clean init

clean:
	-find $(OBJ_DIR)/* | grep -v $(OBJ_DIR)/*.md | xargs rm

init:
	-mkdir $(SRC_DIR)
	-mkdir $(HDR_DIR)
	-mkdir $(OBJ_DIR)
	-mkdir $(LIB_DIR)