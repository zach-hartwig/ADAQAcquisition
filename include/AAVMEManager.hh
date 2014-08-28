#ifndef __AAVMEManager_hh__
#define __AAVMEManager_hh__ 1

#include <TObject.h>

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

class AAVMEManager : public TObject
{
public:
  AAVMEManager();
  ~AAVMEManager();

  static AAVMEManager *GetInstance();

  /////////////////////////////////////
  // Set/get methods for member data //
  /////////////////////////////////////

  bool GetVMEConnectionEstablished() {return VMEConnectionEstablished;}

  // VME board enable booleans

  void SetBREnable(bool E) {BREnable = E;}
  bool GetBREnable() {return BREnable;}

  void SetDGEnable(bool E) {DGEnable = E;}
  bool GetDGEnable() {return DGEnable;}

  void SetHVEnable(bool E) {HVEnable = E;}
  bool GetHVEnable() {return HVEnable;}

  // Get VME board addresses

  void SetDGAddress(long BA) {DGAddress = BA;}
  long GetDGAddress() {return DGAddress;}

  void SetHVAddress(long BA) {HVAddress = BA;}
  long GetHVAddress() {return HVAddress;}

  ClassDef(AAVMEManager, 0);
  
private:
  static AAVMEManager *TheVMEManager;

  // Use booleans and VME addresses
  bool BREnable, DGEnable, HVEnable;
  long DGAddress, HVAddress;
  
  bool VMEConnectionEstablished;
};

#endif
