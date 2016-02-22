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

#ifndef __AATabSlots_hh__
#define __AATabSlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AATabSlots : public TObject
{
public:
  AATabSlots(AAInterface *);
  ~AATabSlots();

  void HandleSettingsTextButtons();
  void HandleConnectionTextButtons();
  void HandlePulserTextButtons();
  void HandleRegisterTextButtons();
  void HandleVoltageTextButtons();

  void HandleCheckButtons();
  void HandleRadioButtons();
  
  ClassDef(AATabSlots, 1);
  
private:
  AAInterface *TI;
};

#endif
