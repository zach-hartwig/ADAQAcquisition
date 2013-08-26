#ifdef __CINT__

#include <vector>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Create dictionaries for the ADAQ classes
#pragma link C++ class ADAQAcquisition+;
#pragma link C++ class ADAQRootMeasParams+;

// is used to readout the digitized waveforms to TTree/TFile
#pragma link C++ class std::vector <short>+;

#endif
