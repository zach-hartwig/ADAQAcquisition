## ADAQAcquisition Source Code Change Log

name: Changelog.md  
date: 10 Jul 15 (last updated)  
auth: Zach Hartwig  
mail: hartwig@psfc.mit.edu  


## Version 1.0 Series

Major new developments in this series include:

### 1.0.1

 - "FPGA Buffer Check" text button now displays the fullness level (as
   percentange) of the FPGA RAM buffer on the digitizer in text and as
   a linear progress bar. When the level exceeds 90%, the bar turns
   bold red to warn the user that data may be lost!

 - Corrected behavior of ADAQ file GUI buttons; GUI button states
   should all correctly reset themselves and ADAQ file written
   regardless of how acquisition if turned on/off, file explicitly
   closed, etc.

 - Correctly implemented what data is stored on ADAQ files based on
   the user's check box selections on the "Persistent storage"
   tab. Proper behavior now results from user choice of "Waveforms",
   "Energy data", and "PSD data".

 - Removed forced use of clang++ compiler in the GNU makefile to
   enable error-less builds on systems using non-clang++ compiler.
