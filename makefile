##
## This is a sample makefile for building Pin tools outside
## of the Pin environment.  This makefile is suitable for
## building with the Pin kit, not a Pin source development tree.
##
## To build the tool, execute the make command:
##
##      make
## or
##      make PIN_HOME=<top-level directory where Pin was installed>
##
## After building your tool, you would invoke Pin like this:
## 
##      $PIN_HOME/pin -t MyPinTool -- /bin/ls
##
##############################################################
#
# User-specific configuration
#
##############################################################

#
# 1. Change PIN_HOME to point to the top-level directory where
#    Pin was installed. This can also be set on the command line,
#    or as an environment variable.
#
PIN_HOME ?= ./..

##############################################################
#
# set up and include *.config files
#
##############################################################

PIN_KIT=$(PIN_HOME)
KIT=1
TESTAPP=$(OBJDIR)cp-pin.exe

CXX=g++

TARGET_COMPILER?=gnu
ifdef OS
    ifeq (${OS},Windows_NT)
        TARGET_COMPILER=ms
    endif
endif

ifeq ($(TARGET_COMPILER),gnu)
    include $(PIN_HOME)/source/tools/makefile.gnu.config
    LINKER?=${CXX}
    CXXFLAGS ?= -Wall -Wno-unknown-pragmas $(DBG) $(OPT)
    PIN=$(PIN_HOME)/pin
endif

ifeq ($(TARGET_COMPILER),ms)
    include $(PIN_HOME)/source/tools/makefile.ms.config
    DBG?=
    PIN=$(PIN_HOME)/pin.bat
endif

##############################################################
# Tools - you may wish to add your tool name to TOOL_ROOTS
##############################################################
#CXXFLAGS+=-pg
#LDFLAGS+=-pg

CXXXMLFLAGS=-O3 -g -DTIXML_USE_TICPP -fPIC
#-std=c++0x
INCLUDES=-I.
TOOL_ROOTS = QUAD
TOOLS = $(TOOL_ROOTS:%=$(OBJDIR)%$(PINTOOL_SUFFIX))

TINYXMLSRCS = ticpp.cpp tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp
Q2XMLSRCS = RenewalFlags.cpp Channel.cpp Q2XMLFile.cpp Exception.cpp $(TINYXMLSRCS)
XMLOBJS = $(Q2XMLSRCS:%.cpp=%.o)

#add the names of more CPP files here for the added functionality in QUAD
# RenewalFlags.cpp is directly included in file
CPPSRCS = BBlock.cpp Utility.cpp
CPPOBJS = $(CPPSRCS:%.cpp=%.oo)
CPPFLAGS = -O3
CPPINCS = -I. 

##############################################################
# build rules
##############################################################
all: tools
tools: $(CPPOBJS) $(XMLOBJS) $(OBJDIR) $(TOOLS) $(OBJDIR)cp-pin.exe
test: $(OBJDIR) $(TOOL_ROOTS:%=%.test)

QUAD.test: $(OBJDIR)cp-pin.exe
      $(MAKE) -k -C QUAD PIN_HOME=$(PIN_HOME)

$(OBJDIR)cp-pin.exe:
	$(CXX) $(PIN_HOME)/source/tools/Tests/cp-pin.cpp $(APP_CXXFLAGS) $(CPPOBJS) $(XMLOBJS) -o $(OBJDIR)cp-pin.exe

$(OBJDIR):
	mkdir -p $(OBJDIR)

# This is added because tracing.cpp is included in QUAD.cpp. This is BAD PRACTICE
# and could be solved by making a QUAD.h, but I do not have time now.
$(OBJDIR)QUAD.o: QUAD.cpp tracing.cpp
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ QUAD.cpp

$(OBJDIR)%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(PIN_CXXFLAGS) ${OUTOPT}$@ $<

%.o: %.cpp
	$(CXX) $(INCLUDES) $(PIN_CXXFLAGS) $(CXXXMLFLAGS) -c $< -o  $@

%.oo: %.cpp
	$(CXX) $(CPPINCS) $(CPPFLAGS) -c $< -o $@

$(TOOLS): $(PIN_LIBNAMES)

$(TOOLS): %$(PINTOOL_SUFFIX) : %.o
	${LINKER} $(PIN_LDFLAGS) $(LINK_DEBUG) ${LINK_OUT}$@ $< $(CPPOBJS) $(XMLOBJS) ${PIN_LPATHS} $(PIN_LIBS) $(DBG) $(LDFLAGS)

## cleaning
clean:
	-rm -rf $(OBJDIR) *.out *.tested *.failed makefile.copy $(XMLOBJS) $(CPPOBJS) *~

