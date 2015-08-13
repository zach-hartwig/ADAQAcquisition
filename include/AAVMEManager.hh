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
#ifndef __CINT__
#include "ADAQVBoard.hh"
#endif

#include "AAInterface.hh"
#include "AASettings.hh"

class AAVMEManager : public TObject
{
public:
  AAVMEManager();
  ~AAVMEManager();

  static AAVMEManager *GetInstance();

  Int_t InitializeBridge();
  Int_t InitializeDigitizer();
  Int_t InitializeHighVoltage();

  bool ProgramDigitizers();

  void SafelyDisconnectVMEBoards();


  /////////////////////////////////////
  // Set/get methods for member data //
  /////////////////////////////////////

  void SetVMEConnectionEstablished(bool CE) {VMEConnectionEstablished = CE;}
  bool GetVMEConnectionEstablished() {return VMEConnectionEstablished;}

  void SetSettingsPointer(AASettings *TS) {TheSettings = TS;}

  // Set/Get methods for VME/USB bridge (BR) settings
  
  void SetBREnable(bool E) {BREnable = E;}
  bool GetBREnable() {return BREnable;}

  void SetBRType(Int_t T) {BRType = T;}
  Int_t GetBRType() {return BRType;}

  // Set/Get methods for the digitizer (DG) settings

  void SetDGEnable(bool E) {DGEnable = E;}
  bool GetDGEnable() {return DGEnable;}

  void SetDGType(Int_t T) {DGType = T;}
  Int_t GetDGType() {return DGType;}

  void SetDGAddress(long BA) {DGAddress = BA;}
  long GetDGAddress() {return DGAddress;}

  void SetDGLinkNumber(Int_t LN) {DGLinkNumber = LN;}
  Int_t GetDGLinkNumber() {return DGLinkNumber;}

  // Set/Get methods for high voltage (HV) settings

  void SetHVEnable(bool E) {HVEnable = E;}
  bool GetHVEnable() {return HVEnable;}

  void SetHVType(Int_t T) {HVType = T;}
  Int_t GetHVType() {return HVType;}

  void SetHVAddress(long BA) {HVAddress = BA;}
  long GetHVAddress() {return HVAddress;}

  void SetHVLinkNumber(Int_t LN) {HVLinkNumber = LN;}
  Int_t GetHVLinkNumber() {return HVLinkNumber;}

  // Public access methods to obtain the board managers

  ADAQBridge *GetBRManager() {return BRMgr;}
  ADAQDigitizer *GetDGManager() {return DGMgr;}
  ADAQHighVoltage *GetHVManager() {return HVMgr;}

  // General purpose VME functions

  void StartHVMonitoring(AAInterface *);
  void StopHVMonitoring();
    

  ClassDef(AAVMEManager, 1);
  
private:
  static AAVMEManager *TheVMEManager;

  // Boolean enable flags
  Bool_t BREnable;
  Int_t BRType;
  int BRIdentifier;

  Bool_t DGEnable;
  Int_t DGType;
  long DGAddress;
  Int_t DGIdentifier, DGLinkNumber, DGCONETNode;

  Bool_t HVEnable;
  Int_t HVType;
  long HVAddress;
  Int_t HVIdentifier, HVLinkNumber;
  
  Bool_t VMEConnectionEstablished;
  Bool_t HVMonitorEnable;

  ADAQBridge *BRMgr;
  ADAQDigitizer *DGMgr;
  ADAQHighVoltage *HVMgr;

  AASettings *TheSettings;
};

#endif
