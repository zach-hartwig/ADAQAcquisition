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

class AAVMEManager : public TObject
{
public:
  AAVMEManager();
  ~AAVMEManager();

  static AAVMEManager *GetInstance();


  void SafelyDisconnectVMEBoards();

  /////////////////////////////////////
  // Set/get methods for member data //
  /////////////////////////////////////

  void SetVMEConnectionEstablished(bool CE) {VMEConnectionEstablished = CE;}
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

  // Get the VME board managers

  ADAQBridge *GetBRManager() {return BRMgr;}
  ADAQDigitizer *GetDGManager() {return DGMgr;}
  ADAQHighVoltage *GetHVManager() {return HVMgr;}

  ClassDef(AAVMEManager, 0);
  
private:
  static AAVMEManager *TheVMEManager;

  // Use booleans and VME addresses
  bool BREnable, DGEnable, HVEnable;
  long DGAddress, HVAddress;
  
  bool VMEConnectionEstablished;

  ADAQBridge *BRMgr;
  ADAQDigitizer *DGMgr;
  ADAQHighVoltage *HVMgr;



  // DGScope objects
  /*
  TGraph *DGScopeWaveform_G[8];
  TH1F *DGScopeSpectrum_H[8];

  TFile *OutputDataFile;
  TTree *WaveformTree;
  bool BranchWaveformTree;
  ADAQRootMeasParams *MeasParams;
  TObjString *MeasComment;
  bool ROOTFileOpen;

  vector<bool> UseCalibrationManager;
  vector<TGraph *> CalibrationManager;
  vector<ADAQChannelCalibrationData> CalibrationData;
  */

  
  // Objects for controlling timed acquisition periods
  // bool AcquisitionTimerEnabled;
  // double AcquisitionTime_Start, AcquisitionTime_Stop;

  // Strings for file names, extensions
  // string DataFileName, DataFileExtension;
  // string SpectrumFileName, SpectrumFileExtension;
  // string GraphicsFileName, GraphicsFileExtension;


};

#endif
