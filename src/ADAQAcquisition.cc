/////////////////////////////////////////////////////////////////////////////////
//
// name: ADAQAcquisition.cc
// date: 28 Aug 14
// auth: Zach Hartwig
// mail: hartwig@psfc.mit.edu
//
// desc:
//
/////////////////////////////////////////////////////////////////////////////////


// ROOT 
#include <TApplication.h>

// C++ 
#include <iostream>
#include <string>
using namespace std;

// ADAQ 
#include "AAVMEManager.hh"
#include "AAInterface.hh"


int main(int argc, char **argv)
{
  // Run ROOT in standalone mode
  TApplication *TheApplication = new TApplication("ADAQAcquisition", &argc, argv);

  // Create the singleton VME manager
  AAVMEManager *TheVMEManager = new AAVMEManager;

  // Create the graphical user interface
  AAInterface *TheInterface = new AAInterface;
  
  // Run the standalone application
  TheApplication->Run();
    
  // Garbage collection ..
  delete TheInterface;
  delete TheVMEManager;
  
  return 0;
}
