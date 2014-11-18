## ADAQAcquisition

### License and disclaimer

### Build instructions  

On Linux, clone into the repository and then use the provided
GNU Makefile to build PDF and Postscript:

```bash
  # Clone ADAQAcquisition source code from GitHub
  git clone https://github.com/ADAQAcquisition/ADAQAcquisition.git

  # Move to the ADAQAcquisition source code directory:
  cd ADAQAcquisition
  
  # To build the binary locally
  make  

  # To cleanup all build files:  
  make clean  
```

### Code dependencies

1. GNU make - The ADAQAcquisition build is controlled using [GNU
make](http://www.gnu.org/software/make/). The makefile provides
automatic compilation of the final binary from the raw C++ source
code, including handling the more complicated inclusion of the various
headers and libraries necessary to complete the build.

2. ROOT (mandatory) - 

3. ADAQ library (mandatory) -

4. CAEN libraries (mandatory) - 
  - CAENVMELib
  - CAENComm
  - CAENDigitizer

5. Boost libraries -

### Directory structure

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



