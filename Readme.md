![AIMS Logo](doc/figures/AIMSLogo_BoldPastelColors.png "Accelerator-based In-situ Materials Surveillance")  
**A**ccelerator-based **I**n-situ **M**aterials **S**urveillance


## ADAQAcquisition ##

ADAQAcquisition is a program that provides a powerful graphical
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
VME-USB/OpticalLink bridge, and pulser boards. ADAQAcquisition
makes extensive use of two primary dependencies: the CAEN libraries
for VME acquisition programming, control, and readout; and the ROOT
C++ data analysis framework.


### License and disclaimer ###

The ADAQAcquisition source code is licensed under the GNU General
Public License v3.0.  You have the right to modify and/or redistribute
this source code under the terms specified in the license,

ADAQAcquisition is provided *without any warranty nor guarantee of
fitness for any particular purpose*. The author(s) shall have no
liability for direct, indirect, or other undesirable consequences of
any character that may result from the use of this source code. This
may include - but is not limited - to irrevocable changes to the
user's firmware, software, hardware, or data. By your use of
ADAQAcquisition, you implicitly agree to absolve the author(s) of
any liability whatsoever. The reader is encouraged to consult the
ADAQAcquisition User's Guide and is advised that the use of this
source code is at his or her own risk.

A copy of the GNU General Public License v3.0 may be found within this
repository at $ADAQACQUISITION/License.md or is available online at
http://www.gnu.org/licenses.


### Obtaining and building the code ###

First, a word on versioning of ADAQAcquisition. Git tags of the form
X.Y.Z are used to indicate stable, production versions of the code
that may be deployed with confidence for general use by the general
user. A change in the X version number indicates a major release that
departs substantially from the previous series, while a change in the
Y version number indicates deployment of major new code features. A
change in the Z number is used to indicate bug fixes and very minor
changes to the codebase. Untagged commits are considered development
versions of the code with no guarantee of stability and should only be
used by developers in non-production situations.

To obtain ADAQAcquisition, you'll need to first clone the repository
from GitHub and then switch to the appropriate git tag version before
building the code.

```bash
  # Clone ADAQAcquisition source code from GitHub:
  git clone https://github.com/zach-hartwig/ADAQAcquisition.git

  # Switch to a tagged production branch. For example:
  cd ADAQAcquisition
  git checkout -b 1.0.0-beta
  
  # To build the binary:
  make  

  # To cleanup all build files and remove the binary:
  make clean  

Finally, add the following lines to your .bashrc file to configure
your environment correctly before running ADAQAcquisition:
     
```bash 
    source /full/path/to/ADAQAcquisition/scripts/setup.sh >& /dev/null
```
Don't forget to open a new terminal for the settings to take effect!


### Code dependencies ###

ADAQAcquisition depends on the following external codes and
libraries. All dependencies, with the exception of the CAEN libraries
which are provided as part of the ADAQ libraries, must be properly
configured on the system prior to building ADAQAcquisition.

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

Zach Hartwig

[Dept. of Nuclear Science and
Engineering](http://web.mit.edu/nse/http://web.mit.edu/nse/) &  
[Plasma Science and Fusion Center](http://www.psfc.mit.edu)  
[Massachusetts Institute of Technology](http://mit.edu)  

phone: +1 617 253 5471  
email: [hartwig@psfc.mit.edu](mailto:hartwig@psfc.mit.edu)  
smail: 77 Massachusetts Ave, NW17-115, Cambridge MA 02139, USA
