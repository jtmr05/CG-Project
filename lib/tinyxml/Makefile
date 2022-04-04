#****************************************************************************
#
# Makefile for TinyXml test.
# Lee Thomason
# www.grinninglizard.com
#
# This is a GNU make (gmake) makefile
#****************************************************************************

# DEBUG can be set to YES to include debugging info, or NO otherwise
DEBUG          := NO

# PROFILE can be set to YES to include profiling info, or NO otherwise
PROFILE        := NO

# TINYXML_USE_STL can be used to turn on STL support. NO, then STL
# will not be used. YES will include the STL files.
TINYXML_USE_STL := NO

#****************************************************************************

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CFLAGS     := -Wall -Wno-format -g -DDEBUG -std=c++17
RELEASE_CFLAGS   := -Wall -Wno-unknown-pragmas -Wno-format -O3 -std=c++17

LIBS		 :=

DEBUG_CXXFLAGS   := ${DEBUG_CFLAGS}
RELEASE_CXXFLAGS := ${RELEASE_CFLAGS}

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CFLAGS       := ${DEBUG_CFLAGS}
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CFLAGS       := ${RELEASE_CFLAGS}
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

ifeq (YES, ${PROFILE})
   CFLAGS   := ${CFLAGS} -pg -O3
   CXXFLAGS := ${CXXFLAGS} -pg -O3
   LDFLAGS  := ${LDFLAGS} -pg
endif

.DEFAULT_GOAL := all

#****************************************************************************
# Preprocessor directives
#****************************************************************************

ifeq (YES, ${TINYXML_USE_STL})
  DEFS := -DTIXML_USE_STL
else
  DEFS :=
endif

#****************************************************************************
# Include paths
#****************************************************************************

#INCS := -I/usr/include/g++-2 -I/usr/local/include
INCS := -Iinclude


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************

CFLAGS   := ${CFLAGS}   ${DEFS}
CXXFLAGS := ${CXXFLAGS} ${DEFS}

#****************************************************************************
# Targets of the build
#****************************************************************************



#****************************************************************************
# Source files
#****************************************************************************

SRCS := $(shell find src/*.cpp)

# Add on the sources for libraries
SRCS := ${SRCS}

OBJS := $(patsubst src/%,%.o,${SRCS})

debug:
	@echo $(OBJS)


all: ${OBJS}
	ar -rcs lib/libtinyxml.a $^


#****************************************************************************
# common rules
#****************************************************************************

# Rules for compiling source files to object files
%.o : src/%
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f ${OBJS}

depend:
	#makedepend ${INCS} ${SRCS}

tinyxml.o: include/tinyxml.h include/tinystr.h
tinyxmlparser.o: include/tinyxml.h include/tinystr.h
tinyxmlerror.o: include/tinyxml.h include/tinystr.h
