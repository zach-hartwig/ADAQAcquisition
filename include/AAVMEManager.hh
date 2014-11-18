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

#ifndef __AAVMEManager_hh__
#define __AAVMEManager_hh__ 1

#include <TObject.h>

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

#include <vector>

class ADAQBridge;
class ADAQDigitizer;
class ADAQHighVoltage;

#include "AAInterface.hh"
#include "AASettings.hh"

class AAVMEManager : public TObject
{
public:
  AAVMEManager();
  ~AAVMEManager();

  static AAVMEManager *GetInstance();

  bool ProgramDigitizers();

  void SafelyDisconnectVMEBoards();


  /////////////////////////////////////
  // Set/get methods for member data //
  /////////////////////////////////////

  void SetVMEConnectionEstablished(bool CE) {VMEConnectionEstablished = CE;}
  bool GetVMEConnectionEstablished() {return VMEConnectionEstablished;}

  void SetSettingsPointer(AASettings *TS) {TheSettings = TS;}
  
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

  // Get the VME board managers

  ADAQBridge *GetBRManager() {return BRMgr;}
  ADAQDigitizer *GetDGManager() {return DGMgr;}
  ADAQHighVoltage *GetHVManager() {return HVMgr;}

  // General purpose VME functions

  void StartHVMonitoring(AAInterface *);
  void StopHVMonitoring();
    

  ClassDef(AAVMEManager, 0);
  
private:
  static AAVMEManager *TheVMEManager;

  // Use booleans and VME addresses
  bool BREnable, DGEnable, HVEnable;
  long DGAddress, HVAddress;
  
  bool VMEConnectionEstablished;
  bool HVMonitorEnable;

  ADAQBridge *BRMgr;
  ADAQDigitizer *DGMgr;
  ADAQHighVoltage *HVMgr;

  AASettings *TheSettings;
};

#endif
