![AIMS Logo](doc/figures/AIMSLogo_BoldPastelColors.png "Accelerator-based In-situ Materials Surveillance")  
**A**ccelerator-based **I**n-situ **M**aterials **S**urveillance


## ADAQAcquisition ##

**ADAQAcquisition** is a program that provides a powerful graphical
user interface (GUI) method for the interacting, programming, and
acquiring digitized waveform data using CAEN high voltage and
acquisition hardware. It provides the user with a full range of
functionality, from high level methods for accomplishing complex tasks
all the way to reading/writing individual registers on the
hardware. In addition to a fully featured digital oscilloscope and
multichannel analyzer, the program provides persistent storage of
waveform data in compressed binary files for offline analysis with its
sister program **ADAQAnalysis**, publication ready graphical output of
waveforms and spectra, and control over VME high voltage,
VME-USB/OpticalLink bridge, and pulser boards. **ADAQAcquisition**
makes extensive use of two primary dependencies: the CAEN libraries
for VME acquisition programming, control, and readout; and the ROOT
C++ data analysis framework.


### License and disclaimer ###

The **ADAQAcquisition** source code is licensed under the GNU General
Public License v3.0.  You have the right to modify and/or redistribute
this source code under the terms specified in the license,

**ADAQAcquisition** is provided *without any warranty nor guarantee of
fitness for any particular purpose*. The author(s) shall have no
liability for direct, indirect, or other undesirable consequences of
any character that may result from the use of this source code. This
may include - but is not limited - to irrevocable changes to the
user's firmware, software, hardware, or data. By your use of
**ADAQAcquisition**, you implicitly agree to absolve the author(s) of
any liability whatsoever. The reader is encouraged to consult the
**ADAQAcquisition** User's Guide and is advised that the use of this
source code is at his or her own risk.

A copy of the GNU General Public License v3.0 may be found within this
repository at $ADAQACQUISITION/License.md or is available online at
http://www.gnu.org/licenses.


### Build instructions ###

**ADAQAcquisition** is designed to be built locally via the provided
GNU makefile, which handles all of the necessary header and library
dependencies. If the user desires installation in a system-wide
location, a Bash script is provided to correctly handle this
operation.


The following lines should first be added to your .bashrc file to
configure your environment correctly:

```bash 
    # ADAQAcquisition configuration
    ADAQACQUISITION_HOME = /full/path/to/ADAQAcquisition
    ADAQACQUISITION_USER = dev # Setting for local install (developer)
    # ADAQACQUISITION_USER = usr # Setting for global install (user)
    source $ADAQACQUISITION_HOME/scripts/setup.sh $ADAQACQUISITION_USER >& /dev/null
```
Don't forget to open a new terminal for the settings to take effect!

On Linux or MacOS, clone into the repository and then use the provided
GNU Makefile to build the **ADAQAcquisition** binary:

```bash
  # Clone ADAQAcquisition source code from GitHub
  git clone https://github.com/ADAQAcquisition/ADAQAcquisition.git

  # Move to the ADAQAcquisition source code directory:
  cd ADAQAcquisition
  
  # To build the binary locally
  make  

  # To cleanup all build files and remove the binary:
  make clean  
```

### Code dependencies ###

**ADAQAcquisition** depends on the following external codes and
libraries. All dependencies, with the exception of the CAEN libraries
which are provided as part of the ADAQ libraries, must be properly
configured on the system prior to building **ADAQAcquisition**.

1. [GNU make](http://www.gnu.org/software/make/)

2. [ROOT](http://root.cern.ch/drupal/)

3. The ADAQ libraries

4. [The CAEN VME libraries](http://www.caen.it/csite/Function.jsp?parent=38&idfun=99)

5. [Boost](http://www.boost.org/)


### Directory structure ###

The ADAQAcquisition directory structure and build system are pretty
straightforward and easy to understand:

  - **bin/**       : Contains final binary

  - **build/**     : Contains transient build files

  - **doc/**       : LaTeX code for the ADAQAcquisition User's Guide

  - **include/**   : C++ header files, ROOT dictionary header file

  - **scripts/**   : Collection of Bash utility scripts

  - **src/**       : C++ source code 

  - **Changelog.md** : List of major content updates and fixes for each Formulary release
  
  - **License.md**   : Markdown version of the GNU GPL v3.0 
  
  - **Makefile**     : GNU makefile for building ADAQAcquisition

  - **Readme.md**  : You're reading it

### Contact ###

Dr. Zachary S Hartwig  

Department of Nuclear Science and Engineering &  
Plasma Science and Fusion Center  
Massachusetts Institute of Technology  

phone: +1 617 253 5471  
email: [hartwig@psfc.mit.edu](mailto:hartwig@psfc.mit.edu)  
smail: 77 Massachusetts Ave, NW17-115, Cambridge MA 02139, USA

