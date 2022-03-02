# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------


#suppress leaving directory messages
MAKEFLAGS += --no-print-directory

#define the generator, engine, bin and lib directories
ENG_DIR := engine
GEN_DIR := generator
UTILS_DIR := utils
BIN_DIR := $(PWD)/bin
LIB_DIR := $(PWD)/lib

FREEGLUT_DIR := $(LIB_DIR)/freeglut
UTILS_LIB_DIR := $(PWD)/$(UTILS_DIR)/lib

#define the compiler
CXX := g++

#Windows
ifeq (Windows_NT, $(OS))
	IS_WIN = yes
endif

#compiler flags
FLAGS := -Wall -Wextra -Wsign-conversion -Iinclude -I$(PWD)/$(UTILS_DIR)/include
LINKER_FLAGS := -L$(UTILS_LIB_DIR) -lutils

ifdef IS_WIN
	FLAGS += -I$(FREEGLUT_DIR)/include
	LINKER_FLAGS += -L$(FREEGLUT_DIR)/lib/x64 -lopengl32 -lfreeglut -lglu32
else
	LINKER_FLAGS += -lGLU -lglut -lGL
endif



export CXX FLAGS LINKER_FLAGS BIN_DIR



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: utils generator engine

build: clean all

.PHONY: utils
generator: utils
	make -C $(GEN_DIR)

engine: utils
	make -C $(ENG_DIR)

utils:
	make -C $(UTILS_DIR)


#'clean' doesn't represent actual file generating recipes
.PHONY: clean

clean:
	-find $(BIN_DIR)/* | grep -v $(BIN_DIR)/*.md | xargs rm
	-make -C $(ENG_DIR) clean
	-make -C $(GEN_DIR) clean
	-make -C $(UTILS_DIR) clean

