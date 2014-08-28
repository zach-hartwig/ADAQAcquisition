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
#include "AAInterface.hh"

// The mandatory C++ main function
int main(int argc, char **argv)
{
  // Create a standalone application that runs outside of a ROOT seesion
  TApplication *TheApplication = new TApplication("ADAQAcquisition", &argc, argv);
  
  // Create variables for width and height of the top-level GUI window
  int Width = 1125;
  int Height = 840;

  // If the user specifies "small" for the first command line
  // arguments then change the width and height settings
  if(argc==2){
    string arg1 = argv[1];
    if(arg1 == "small"){
      Width = 980;
      Height = 650;
    }
    else
      cout << "Error! Only argument allowed is 'small'!\n" << endl;
  }
  
  // Create an object of type ADAQAcquisition and connect its "CloseWindow" function
  AAInterface *MainFrame = new AAInterface(Width, Height);
  //MainFrame->Connect("CloseWindow()", "AAInterface", MainFrame, "HandleDisconnectAndTerminate(bool)");
  
  // Run the standalone application
  TheApplication->Run();
    
  // Clean up memory upon completion
  delete MainFrame;
  
  return 0;
}
