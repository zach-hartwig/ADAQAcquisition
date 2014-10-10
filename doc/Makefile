#********************************************************************
#
#  name: Makefile                  
#  date: 10 Oct 14
#  auth: Zach Hartwig              
#  mail: hartwig@psfc.mit.edu
#
#  desc: GNU makefile for building the ADAQAcquisition User's Guide from
#        the LaTeX source code
#
#  dpnd: Standard LaTeX installation
#
#  To build the manual
#  $ make 
#
#  To clean the bin/ and build/ directories
#  $ make clean
#
#********************************************************************

#********#
# MACROS #
#********#

# Define the directory for transient files
BUILDDIR = build/

# Define the final PDF target
PDF = ADAQAcquisition_UsersGuide.pdf

# Define intermediate build files
PS = $(BUILDDIR)/ADAQAcquisition_UsersGuide.ps
DVI = $(BUILDDIR)/ADAQAcquisition_UsersGuide.dvi

# Define the main LaTeX source file
TEXMAIN = ADAQAcquisition_UsersGuide.tex

# Get all of the LaTeX dependency files
TEXFILES = $(wildcard *.tex)

# Define local directory for custom LaTeX packages
TEXMF=$(PWD)/texmf///:
#TEXMFCMD='export TEXINPUTS=$(TEXMF)'


#*******#
# RULES #
#*******#

$(PDF) : $(PS)
	ps2pdf $< $@

$(PS) : $(DVI)
	dvips -o $@ $<

$(DVI) : $(TEXFILES)
#	export TEXINPUTS=$(TEXMF); latex --output-directory=$(BUILDDIR) $(TEXMAIN)
#	cp $(REF) $(BUILDDIR)
#	cd $(BUILDDIR);	bibtex references
	export TEXINPUTS=$(TEXMF); latex --output-directory=$(BUILDDIR) $(TEXMAIN)
	export TEXINPUTS=$(TEXMF); latex --output-directory=$(BUILDDIR) $(TEXMAIN)

.PHONY:
clean:
	@echo -e "\nCleaning up the LaTeX build directory and PDF file ..."
	@rm $(BUILDDIR)/* $(PDF) -f
	@echo -e ""
