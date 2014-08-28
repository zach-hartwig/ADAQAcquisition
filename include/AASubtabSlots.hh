#ifndef __AASubtabSlots_hh__
#define __AASubtabSlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AASubtabSlots : public TObject
{
public:

  AASubtabSlots(AAInterface *);
  ~AASubtabSlots();

  ClassDef(AASubtabSlots, 0);
  
private:
  AAInterface *TheInterface;
};

#endif
