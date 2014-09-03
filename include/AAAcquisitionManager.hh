#ifndef __AAAcquisitionManager_hh__
#define __AAAcquisitionManager_hh__ 1

#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TGraph.h>

#ifndef __CINT__
#include <boost/cstdint.hpp>

#include "ADAQDigitizer.hh"
#endif

#include <vector>
#include <string>
using namespace std;

#include "ADAQRootClasses.hh"

#include "AATypes.hh"
#include "AAInterface.hh"
#include "AASettings.hh"

class AAAcquisitionManager : public TObject
{
public:
  AAAcquisitionManager();
  ~AAAcquisitionManager();

  static AAAcquisitionManager *GetInstance();

  void StartAcquisition();
  void StopAcquisition();

  void PreAcquisition();

  void CreateWaveformTreeBranches();


  void SetAcquisitionEnable(bool ATE) {AcquisitionEnable = true;}
  bool GetAcquisitionEnable() {return AcquisitionEnable;}
  
  void SetAcquisitionTimerEnable(bool ATE) {AcquisitionTimerEnable = ATE;}
  bool GetAcquisitionTimerEnable() {return AcquisitionTimerEnable;}

  void SetAcquisitionTimeStart(double T) {AcquisitionTimeStart = T;}
  void SetAcquisitionTimeStop(double T) {AcquisitionTimeStop = T;}
  
  void SetInterfacePointer(AAInterface *TI) {TheInterface = TI;}
  void SetSettingsPointer(AASettings *TS) {TheSettings = TS;}

  ClassDef(AAAcquisitionManager, 0);
  
private:
  static AAAcquisitionManager *TheAcquisitionManager;

  bool AcquisitionEnable;

  //Objects for controlling timed acquisition periods
  bool AcquisitionTimerEnable;
  double AcquisitionTimeStart, AcquisitionTimeStop;
  time_t AcquisitionTimeNow, AcquisitionTimePrev;

#ifndef __CINT__
  // Variables for CAEN digitizer readout
  char *EventPointer;
  CAEN_DGTZ_EventInfo_t EventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *EventWaveform;

  // Variables for PC buffer readout
  char *Buffer;
  uint32_t BufferSize, ReadSize, NumEvents;
  vector<vector<uint16_t> > Waveforms;
#endif
  
  vector<int> BaselineStart, BaselineStop, BaselineLength;
  vector<double> BaselineValue;

  vector<double> Polarity;

  int LLD, ULD;
  double SampleHeight, TriggerHeight;
  double PulseHeight, PulseArea;

  vector<bool> CalibrationEnable;
  vector<TGraph *> CalibrationCurves;
  vector<ADAQChannelCalibrationData> CalibrationData;
  
  vector<TH1F *> Spectrum_H;

  bool WriteWaveformToTree;
  bool BranchWaveformTree;


  TFile *OutputDataFile;
  TTree *WaveformTree;

  ADAQRootMeasParams *MeasParams;
  TObjString *MeasComment;
  bool ROOTFileOpen;
  
  // Strings for file names, extensions
  string DataFileName, DataFileExtension;
  string SpectrumFileName, SpectrumFileExtension;
  string GraphicsFileName, GraphicsFileExtension;

  AAInterface *TheInterface;
  AASettings *TheSettings;


};

#endif
