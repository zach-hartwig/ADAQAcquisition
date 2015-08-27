## ADAQAcquisition Source Code Change Log

name: Changelog.md  
date: 26 Aug 15 (last updated)  
auth: Zach Hartwig  
mail: hartwig@psfc.mit.edu  

## Version 1.2 Series

Major new developments in this series include:

 - Implementation of digitizer control and data readout using CAEN
   DPP-PSD firmware. The core functionality is fully implemented and
   tested. Readout of data into ADAQ files is supported via the
   ADAQReadoutManager class. Note that ADAQAcquisition now fully
   supports both CAEN standard *and* DPP-PSD firwmare.

 - The trigger time stamp is now correctly readout with rollover
   correction for very long data acquisition. The correct time stamp
   is stored within the ADAQ file.

 - The VME connection tab widgets now fully specify how the remaining
   four major tabs should be constructed, i.e. the interface now
   dynamically builds itself depending on the exact hardware that the
   user has specified. This ensures the correct firmware-specific
   widgets, number of channels, default parameters, etc. are built at
   run time.

 - The acquisition loop in the AAAcquisitionManager class has been
   restructured to accomodate DPP-PSD and optimized for readout
   efficiency.


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
