/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                           Copyright (C) 2012-2015                           //
//                 Zachary Seth Hartwig : All rights reserved                  //
//                                                                             //
//      The ADAQAcquisition source code is licensed under the GNU GPL v3.0.    //
//      You have the right to modify and/or redistribute this source code      //      
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at $ADAQACQUISITION/License.txt.     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
//
// name: RootLinkDef.h
// date: 07 Oct 14
// auth: Zach Hartwig
// mail: hartwig@psfc.mit.edu
// desc: Mandatory header file for ROOT dictionary generation
///////////////////////////////////////////////////////////////////////////////// 

#ifdef __CINT__

#include <vector>
#include <stdint.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// ADAQAcquisition classes
#pragma link C++ class AAAcquisitionManager+;
#pragma link C++ class AAChannelSlots+;
#pragma link C++ class AADisplaySlots+;
#pragma link C++ class AAGraphics+;
#pragma link C++ class AAInterface+;
#pragma link C++ class AASettings+;
#pragma link C++ class AASubtabSlots+;
#pragma link C++ class AATabSlots+;
#pragma link C++ class AAVMEManager+;

// Create a special vector of uint16_t's. This type is used for
// storing digitized waveform information and is necessary to define
// here to be compatible with the ROOT-agnostic ADAQ libraries
// (otherwise, we could simply have used the ROOT vector<Short_t> type)
#pragma link C++ class std::vector<uint16_t>+;

// ADAQ classes
#pragma link C++ class ADAQRootMeasParams+;

#endif
