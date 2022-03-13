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
BIN_DIR 		:= $(PWD)/bin
LIB_DIR 		:= $(PWD)/lib

UTILS_DIR		:= $(LIB_DIR)/utils
FREEGLUT_DIR 	:= $(LIB_DIR)/freeglut
TINY_XML_DIR 	:= $(LIB_DIR)/tinyxml


export UTILS_DIR FREEGLUT_DIR TINY_XML_DIR


#Windows
ifeq (Windows_NT, $(OS))
	IS_WIN = yes
endif

#compiler flags
FLAGS 			:= -Wall -Wextra -Wsign-conversion -Iinclude


export CXX FLAGS BIN_DIR




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
	-find $(BIN_DIR)/* | grep -v $(BIN_DIR)/*.md | xargs rm
	-make -C $(ENG_DIR) clean
	-make -C $(GEN_DIR) clean
	-make -C $(shell realpath --relative-to . $(UTILS_DIR)) clean
	-make -C $(shell realpath --relative-to . $(TINY_XML_DIR)) clean

