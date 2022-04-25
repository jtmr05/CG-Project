# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------

#suppress leaving directory messages
MAKEFLAGS 		+= --no-print-directory



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

export BIN_DIR UTILS_DIR FREEGLUT_DIR TINY_XML_DIR GLEW_DIR



#archive-maintaining program
AR				:= ar -rcs

#compiler
CXX 			:= g++

#compiler flags
CXXFLAGS		:= -Wall -Wextra -Wsign-conversion -std=c++17 -g

#Windows
ifeq (Windows_NT, $(OS))
	IS_WIN = yes
endif

export AR CXX CXXFLAGS IS_WIN




#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: tinyxml utils generator engine

build: clean all

generator: utils
	make -C $(GEN_DIR)

engine: tinyxml utils
	make -C $(ENG_DIR)

.PHONY: utils
utils:
	make -C $(shell realpath --relative-to . $(UTILS_DIR))

.PHONY: tinyxml
tinyxml:
	make -C $(shell realpath --relative-to . $(TINY_XML_DIR))



#'clean' doesn't represent an actual file generating recipe
.PHONY: clean

clean:
	-make -C $(ENG_DIR) clean
	-make -C $(GEN_DIR) clean
	-make -C $(shell realpath --relative-to . $(UTILS_DIR)) clean
	-make -C $(shell realpath --relative-to . $(TINY_XML_DIR)) clean
	-find $(RSR_DIR)/* | grep .3d | xargs rm
	-rm *.stackdump
	-rm -rf $(BIN_DIR)