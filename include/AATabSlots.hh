#ifndef __AATabSlots_hh__
#define __AATabSlots_hh__ 1

#include <TObject.h>

class AAInterface;

class AATabSlots : public TObject
{
public:
  AATabSlots(AAInterface *);
  ~AATabSlots();

  void HandleCheckButtons();
  void HandleConnectionTextButtons();
  void HandleRegisterTextButtons();
  void HandlePulserTextButtons();
  void HandleVoltageTextButtons();
  
  ClassDef(AATabSlots, 0);
  
private:
  AAInterface *TI;
};

#endif
