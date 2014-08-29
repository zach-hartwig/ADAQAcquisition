#ifndef __AAChannelSlots_hh__
#define __AAChannelSlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AAChannelSlots : public TObject
{
public:

  AAChannelSlots(AAInterface *);
  ~AAChannelSlots();

  void HandleNumberEntries();

  ClassDef(AAChannelSlots, 0);
  
private:
  AAInterface *TI;
};

#endif
