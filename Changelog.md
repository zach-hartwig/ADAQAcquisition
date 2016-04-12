## ADAQAcquisition Source Code Change Log

name: Changelog.md  
date: 12 Apr 16 (last updated)
auth: Zach Hartwig  
mail: hartwig@psfc.mit.edu

## Version 1.6 Series

### 1.6.7

 - Locking channel settings together for pre-trigger on DT5790 since
   this the pre-trigger is a global setting on this device


### 1.6.6

 - Preventing seg fault triggers when the user attempts to calibrate
   digitizer ADCs without connecting to a digitizer

 - Properly storing uncalibrated energy data to the ADAQ file
   regardless of whether spectrum is calibrated/uncalibrated


### 1.6.5

 - Properly re-enabling the acquisition timer setting widget after
   finishing a timed acquisition with data written to ADAQ file

 - Correctly setting the range to apply digitizer channel 0 settings
   to when the user has enabled the channel lock-to-0 feature
   

### 1.6.4

 - Added basic ability to temperature-calibrate digitizer ADCs from
   the "VME connection" tab for x725 and x730 family of digitizers
 
 - Enabling digitizer channel 0 settings to be automatically applied
   to a user-selected range of channels. Greatly eases assigning
   settings for many-channel digitizers.

 - A number of important bugs have been fixed:
   - Correctly allocating number of channels in AASettings class
   - Fixing ability to plot waveforms in STD firmware
   - Enabling HV tab settings to be loaded from settings file
   - Enabling digitizer FPGA buffer check for CAEN DPP-PSD firmware
   

### 1.6.3

 - Apply DPP-PSD channel-specific settings regardless of whether the
   channel is enabled. This prevents "dual channel setting" boards
   (e.g Ch0 value applied to Ch0 and Ch1) from inducing segaults when
   th primary channel (e.g. Ch0) is not enabled. Future work will be
   attempt to make this behavior to the user in the interface.


### 1.6.2

 - Correctly settings CAEN device-specific values for DPP-PSD firmware
   charge sensitivity for x725/x730 family of digitizers


### 1.6.1

 - Correctly casting the CAEN DPP-PSD short charge into a 16-bit
   integer when reading out waveforms with DPP-PSD list mode. It was
   previously reading out into a 15-bit integer, which resulting in
   1/2 the use of the dynamic input.


### 1.6.0

 - New capability to save/load all interface settings to a new
   ADAQAcquistion settings file (*.acq.root). Settings file can be
   loaded from within the program via the new 'Settings' tab or
   specified as the first command line argument at startup. Settings
   can be optionally automatically saved during the session

 - Minor optimization of waveform plotting for efficiency

 - A large number of bugs have been fixed:
   - Fixed the inability to _not_ save waveforms to ADAQ file 
   - Correcting seg fault when DPP-PSD is used with acquisition time
   - Limiting 2**16 max readout into CAEN DPP-PSD list mode only operation
   - Corrected the device-specific baseline calculation maps for CAEN DPP-PSD
   - Allocated correct numbers of waveforms dynamically for the linked device
   - Enable PSD histogram to update when plotting in tail/total Y-axis
   - Disabling PSD histogram widgets during acquisition
   - Corrected default ADAQ file name when none chosen in the file dialog
   - Corrected strange compilation bug for Ubuntu 14.04LTS systems


## Version 1.4 Series

### 1.4.1

 - Fixing a number of seg. fault issues for correct storage of button
   settings under ROOT6 behavior

 - Preventing user from closing ADAQ file while data is being stored
   to prevent seg. fault and file corruption

 - Correctly reenabling the data storage file set button after a data
   acquisition run finishes when using the acquisition timer


### 1.4.0

 - Upgraded for compatibility with ROOT6 (v6.06.00); ADAQAcquisition
   tagged version 1.2.4 is last compatible with ROOT5 (v5.34.30)

 - Increased readout bit depth from 2**15 to 2**16 for FPGA-processed
   data readout using DPP-PSD

 - Corrected state setting of widgets under various use cases

 - Corrected default ADAQ file name to be "DefaultData.adaq.root"


## Version 1.2 Series

### 1.2.4

 - New settings, acquisition corrections to properly handle DPP-PSD on
   x725/x730 boards, which do not support 'Oscilloscope' mode.


### 1.2.3

 - Enabled initial support for the CAEN V1725 digitizer board,
   including both the 8 and 16 channel versions.


### 1.2.2

 - Bug fix to ensure that non-triggered but enabled channels register
   a timestamp of '0'; addresses concern about having repeated time
   stamps stored in ADAQ file

 - Implentation of user-setting for DPP-PSD trigger holdoff global

 - Adding C++ <cmath> include where necessary for compilation on
   Ubuntu systems

 - Prevention of waveform plotting in PSD list mode

 - Reset canvas viewing sliders when switching display types

 - Updating default settins
 

### 1.2.1

 - Bug fixing and complete optimization for digitizer programming and
   readout of waveforms when using DPP-PSD firmware.

 - Implementation of decimal formated display (in addition to the hex
   and binary) for device register readout

 - Enabling the use of the PSD long integral as the pulse area for
   spectroscopy, data, readout, etc

 - Correctly set the maximum limits for HV voltage widgets based on
   the maximum voltage values determined by ADAQHighVoltage class

 - Enabling setting spectral bin limits in floats not just integers


### 1.2.0

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
