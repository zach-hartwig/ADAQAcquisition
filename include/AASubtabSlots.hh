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

  ClassDef(AASubtabSlots, 0);
  
private:
  AAInterface *TI;
};

#endif
