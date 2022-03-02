# -----------//-----------
# This is a C++ makefile.|
# -----------//-----------


#suppress leaving directory messages
MAKEFLAGS += --no-print-directory

#define the generator, engine, bin and lib directories
GEN_DIR := generator
ENG_DIR := engine
BIN_DIR := $(PWD)/bin
LIB_DIR := $(PWD)/lib
HDR_DIR := include   #local dir for subdirectories

FREEGLUT_DIR := $(LIB_DIR)/freeglut

#define the compiler
CXX := g++

#Windows
ifeq (Windows_NT, $(OS))
	IS_WIN = yes
endif

#compiler flags
FLAGS := -Wall -Wextra -Wsign-conversion -I$(HDR_DIR)

ifdef IS_WIN
	FLAGS += -I$(FREEGLUT_DIR)/include
	LINKER_FLAGS := -L$(FREEGLUT_DIR)/lib/x64 -lopengl32 -lfreeglut -lglu32
else
	LINKER_FLAGS := -lGLU -lglut -lGL
endif



export CXX FLAGS LINKER_FLAGS BIN_DIR



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL := all

all: generator engine

build: clean all

.PHONY: generator engine
generator:
	make -C $(GEN_DIR)

engine:
	make -C $(ENG_DIR)



#'clean' doesn't represent actual file generating recipes
.PHONY: clean

clean:
	-find $(BIN_DIR)/* | grep -v $(BIN_DIR)/*.md | xargs rm
	-make -C $(ENG_DIR) clean
	-make -C $(GEN_DIR) clean
