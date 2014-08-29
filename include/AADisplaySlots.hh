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

  ClassDef(AADisplaySlots, 0);
  
private:
  AAInterface *TI;
};

#endif
