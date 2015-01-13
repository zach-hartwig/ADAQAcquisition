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

#ifndef __AASubtabSlots_hh__
#define __AASubtabSlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AASubtabSlots : public TObject
{
public:

  AASubtabSlots(AAInterface *);
  ~AASubtabSlots();

  void HandleCheckButtons();
  void HandleComboBoxes(int, int);
  void HandleNumberEntries();
  void HandleRadioButtons();
  void HandleTextButtons();

  ClassDef(AASubtabSlots, 1);
  
private:
  AAInterface *TI;

  string WaveformFileName;
  string SpectrumFileName;
  string CanvasFileName;
};

#endif
