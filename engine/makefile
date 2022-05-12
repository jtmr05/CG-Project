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
BIN 			:= $(BIN_DIR)/engine



#compiler flags
CXXFLAGS		+= -I$(INC_DIR) -I$(UTILS_DIR)/include -I$(TINYXML_DIR)/include

#linker flags
LDFLAGS			:= -L$(UTILS_DIR)/lib -L$(TINYXML_DIR)/lib

#linker libraries
LDLIBS			:= -lutils -ltinyxml

ifdef IS_WIN
	CXXFLAGS 	+= -I$(FREEGLUT_DIR)/include -I$(GLEW_DIR)/include -I$(DEVIL_DIR)/include
	LDFLAGS 	+= -L$(FREEGLUT_DIR)/lib/x64 -L$(GLEW_DIR)/lib/x64 -L$(DEVIL_DIR)/lib
	LDLIBS		+= -lopengl32 -lfreeglut -lglu32 -lglew32 -ldevil -lilu -lilut
else
	LDLIBS	 	+= -lGLU -lGL -lglut -lGLEW -lILUT -lILU -lIL
endif



#make default goal (using make with no specified recipe)
.DEFAULT_GOAL 	:= all

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
