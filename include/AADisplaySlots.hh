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

#ifndef __AADisplaySlots_hh__
#define __AADisplaySlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AADisplaySlots : public TObject
{
public:

  AADisplaySlots(AAInterface *);
  ~AADisplaySlots();

  void HandleTextButtons();
  void HandleDoubleSliders();
  void HandleSliderPointers();

  ClassDef(AADisplaySlots, 1);
  
private:
  AAInterface *TI;
};

#endif
