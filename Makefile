#********************************************************************
#
#  name: Makefile                  
#  date: 26 Jan 15
#  auth: Zach Hartwig              
#  mail: hartwig@psfc.mit.edu
#
#  desc: GNU makefile for building the ADAQAcquisition binary
#
#  dpnd: The build system requires the following dependencies:
#        -- ROOT 
#        -- ADAQ libraries (ADAQControl, ADAQReadout)
#        -- Boost libraries (boost-thread)
# 
#  To build the binary
#  $ make 
#
#  To clean the bin/ and build/ directories
#  $ make clean
#
#********************************************************************

#**** MACRO DEFINITIONS ****#

# Include the Makefile for ROOT-based projects
RC:=root-config
ROOTSYS:=$(shell $(RC) --prefix)
ROOTMAKE:=$(ROOTSYS)/etc/Makefile.arch
include $(ROOTMAKE)

# Enable C++11 features
CXXFLAGS += -std=c++11

# Specify the the binary, build, and source directories
BUILDDIR = build
BINDIR = bin
SRCDIR = src

# Specify header files directory. Note that this must be an absolute
# path to ensure the ROOT dictionary files can find the headers
INCLDIR = $(PWD)/include

# Specify all header files
INCLS = $(INCLDIR)/*.hh

# Specify all object files (to be built in the build/ directory)
SRCS = $(wildcard $(SRCDIR)/*.cc)
TMP = $(patsubst %.cc,%.o,$(SRCS))
OBJS = $(subst src/,build/,$(TMP))

# Add the mandatory ROOT dictionary object file
OBJS += $(BUILDDIR)/ADAQAcquisitionDict.o

# Add various compiler flags
CXXFLAGS += -w -I$(INCLDIR) -I$(ADAQHOME)/include

# Add linker flags for the ADAQ libraries
LDFLAGS+=-L$(ADAQHOME)/lib/$(HOSTTYPE) -lADAQControl -lADAQReadout

# Add linker flags for CAEN libraries (architecture dependent). Note
# that these libraries are PROVIDED by the ADAQ code
LDFLAGS+=-L$(ADAQHOME)/lib/$(HOSTTYPE) -lCAENVME -lCAENComm -lCAENDigitizer -lncurses -lc -lm -lrt

# Define the target binary
TARGET = $(BINDIR)/ADAQAcquisition

#***************#
#**** RULES ****#
#***************#

#*************************#
# Rules to build the binary

$(TARGET) : $(OBJS) 
	@echo -e "\nBuilding $@ ..."
	$(CXX) -g -o $@ $^ $(LDFLAGS) $(ROOTGLIBS)
	@echo -e "\n$@ build is complete!\n"

$(BUILDDIR)/%.o : $(SRCDIR)/%.cc $(INCLS)
	@echo -e "\nBuilding object file '$@' ..."
	$(CXX) $(CXXFLAGS) -c -o $@ $<

#***********************************************#
# Rules to generate the necessary ROOT dictionary


$(BUILDDIR)/ADAQAcquisitionDict.o : $(BUILDDIR)/ADAQAcquisitionDict.cc
	@echo -e "\nBuilding '$@' ..."
	$(CXX) -g $(CXXFLAGS) -c -o $@ $<

# Generate the necessary ROOT dictionaries
$(BUILDDIR)/ADAQAcquisitionDict.cc : $(INCLS) $(INCLDIR)/RootLinkDef.h
	@echo -e "\nGenerating ROOT dictionary '$@' ..."
	rootcling -f $@ -c -I$(ADAQHOME)/include $^
	@cp $(BUILDDIR)/*.pcm $(BINDIR)

# Clean the directory of all build files and binaries
.PHONY: 
clean:
	@echo -e "\nCleaning up the build and binary ..."
	rm -f $(BUILDDIR)/*.o *.d $(BUILDDIR)/*Dict.* $(TARGET)
	@echo -e ""

# Useful notes for the uninitiated:
#
# <target> : <dependency list>
#  --TAB-- <rule>
#
# "$@" == subst. the word to the left of the ":"
# "$^" == subst. the words to the right of ":"
# "$<" == subst. first item in dependency list
