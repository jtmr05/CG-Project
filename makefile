# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------


#suppress leaving directory messages
MAKEFLAGS 		+= --no-print-directory

#compiler
CXX 			:= g++

#directories
ENG_DIR 		:= engine
GEN_DIR 		:= generator
RSR_DIR			:= resources
BIN_DIR 		:= $(PWD)/bin
LIB_DIR 		:= $(PWD)/lib

UTILS_DIR		:= $(LIB_DIR)/utils
FREEGLUT_DIR 	:= $(LIB_DIR)/freeglut
TINY_XML_DIR 	:= $(LIB_DIR)/tinyxml
GLEW_DIR		:= $(LIB_DIR)/glew


export UTILS_DIR FREEGLUT_DIR TINY_XML_DIR GLEW_DIR


#Windows
ifeq (Windows_NT, $(OS))
	IS_WIN = yes
endif

#compiler flags
FLAGS 			:= -Wall -Wextra -Wsign-conversion -Iinclude -std=c++17


export CXX FLAGS BIN_DIR IS_WIN




#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: tinyxml utils generator engine

build: clean all

.PHONY: utils tinyxml
generator: utils
	make -C $(GEN_DIR)

engine: tinyxml utils
	make -C $(ENG_DIR)

utils:
	make -C $(shell realpath --relative-to . $(UTILS_DIR))

#in case there's need to recompile
tinyxml:
	make -C $(shell realpath --relative-to . $(TINY_XML_DIR))

#'clean' doesn't represent actual file generating recipes
.PHONY: clean

clean:
	-make -C $(ENG_DIR) clean
	-make -C $(GEN_DIR) clean
	-make -C $(shell realpath --relative-to . $(UTILS_DIR)) clean
	-make -C $(shell realpath --relative-to . $(TINY_XML_DIR)) clean
	-find $(RSR_DIR)/* | grep .3d | xargs rm
	-rm *.stackdump

shred: clean
	-find $(BIN_DIR)/* | grep -v **/*.md | xargs rm
	-find $(UTILS_DIR)/* | grep -F .a | xargs rm
	-find $(TINY_XML_DIR)/* | grep -F .a | xargs rm