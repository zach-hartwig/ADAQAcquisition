/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                           Copyright (C) 2012-2016                           //
//                 Zachary Seth Hartwig : All rights reserved                  //
//                                                                             //
//      The ADAQAcquisition source code is licensed under the GNU GPL v3.0.    //
//      You have the right to modify and/or redistribute this source code      //      
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at $ADAQACQUISITION/License.txt.     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#ifndef __AAInterface_hh__
#define __AAInterface_hh__ 1

#include <TColor.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGFrame.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TObject.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TApplication.h>
#include <TGDoubleSlider.h>
#include <TGTripleSlider.h>
#include <TGTextView.h>
#include <TGFileDialog.h>
#include <TGProgressBar.h>

#include <vector>
#include <map>
#include <string>
using namespace std;

#include "ADAQRootClasses.hh"

#include "AATypes.hh"
#include "AASettings.hh"
class AAChannelSlots;
class AADisplaySlots;
class AASubtabSlots;
class AATabSlots;
class AAVMEManager;
class AAAcquisitionManager;

// Define the maximum number of digitizer channels supported by the
// ADAQ framework
#define MAX_DG_CHANNELS 16


class AAInterface : public TGMainFrame
{
  friend class AAChannelSlots;
  friend class AADisplaySlots;
  friend class AASubtabSlots;
  friend class AATabSlots;

public:

  AAInterface(Bool_t, string);
  ~AAInterface();

  void BuildPrimaryFrames();
  void BuildSecondaryFrames();
  
  void CreateTopLevelFrames();
  void FillSettingsFrame();
  void FillConnectionFrame();
  void FillRegisterFrame();
  void FillPulserFrame();
  void FillVoltageFrame();
  void FillAcquisitionFrame();
  
  void UpdateAQTimer(int);
  void UpdateAfterAQTimerStopped(bool);
  void UpdateAfterCalibrationPointAdded(int);
  void UpdateHVMonitors(int, int, int);
  void UpdateChannelSettingsToChannelZero();

  string CreateFileDialog(const char *[], EFileDialogMode);

  void HandleDisconnectAndTerminate(bool = true);

  // Methods for handling widget settings
  void SaveSettings();
  void SaveActiveSettings();
  void SaveSettingsToFile();
  void LoadSettingsFromFile();
  
  // Methods to handle widget states
  void SetVoltageChannelWidgetState(int, bool);
  void SetVoltageWidgetState(bool, EButtonState);
  void SetAcquisitionWidgetState(bool, EButtonState);
  void SetCalibrationWidgetState(bool, EButtonState);
  void SetTitlesWidgetState(bool, EButtonState);

private:

  ///////////////////////////
  // General use variables //
  ///////////////////////////
  
  Bool_t InterfaceBuildComplete;
  
  // Dimensions for interface window
  Int_t DisplayWidth, DisplayHeight;
  Int_t ButtonForeColor, ButtonBackColorOn, ButtonBackColorOff;
  
  // File name for interface settings storage
  string SettingsFileName;
  Bool_t AutoSaveSettings, AutoLoadSettings;
  
  // Class to hold all interface widget settings
  AASettings *TheSettings;

  // Variables for use with high voltage widgets
  vector<string> HVChLabels;
  vector<Int_t> HVChPower_TB_ID_Vec;
  map<Int_t, Int_t> HVChPower_TB_ID_Map;

  // Channel-specific variables for acquisition setting widgets
  vector<string> DGChannelLabels;

  vector<Int_t> DGChEnable_CB_ID_Vec;

  vector<Int_t> DGChDCOffset_NEL_ID_Vec;

  vector<Int_t> DGChTriggerThreshold_NEL_ID_Vec;
  map<Int_t, Int_t> DGChTriggerThreshold_NEL_ID_Map;
  
  vector<Int_t> DGChRecordLength_NEL_ID_Vec;
  map<Int_t, Int_t> DGChRecordLength_NEL_ID_Map;
  
  vector<Int_t> DGChPreTrigger_NEL_ID_Vec;
  map<Int_t, Int_t> DGChPreTrigger_NEL_ID_Map;

  // Channel-specific variables for analysis setting widgets
  vector<int> DGChBaselineCalcMin_NEL_ID_Vec;
  vector<int> DGChBaselineCalcMax_NEL_ID_Vec;
  vector<int> DGChPSDTotalStart_NEL_ID_Vec;
  vector<int> DGChPSDTotalStop_NEL_ID_Vec;
  vector<int> DGChPSDTailStart_NEL_ID_Vec;
  vector<int> DGChPSDTailStop_NEL_ID_Vec;

  // Object to convert numeric color to pixel color
  TColor *ColorManager;
  
  // Variables for handling boards/devices
  const Int_t NumBoards;
  enum{zBR, zDG, zHV};

  // "Slot" classes for handling widget signals
  AAChannelSlots *ChannelSlots;
  AADisplaySlots *DisplaySlots;
  AASubtabSlots *SubtabSlots;
  AATabSlots *TabSlots;

  /////////////////////////////
  // ROOT GUI widget objects //
  /////////////////////////////

  ////////////
  // Tab frame

  TGTab *TopLevelTabs;
  TGVerticalFrame *TopFrame;
  TGCompositeFrame *ConnectionTab, *ConnectionFrame;
  TGCompositeFrame *RegisterTab, *RegisterFrame;
  TGCompositeFrame *PulserTab, *PulserFrame;
  TGCompositeFrame *VoltageTab, *VoltageFrame;
  TGCompositeFrame *AcquisitionTab, *AcquisitionFrame;
  TGCompositeFrame *SettingsTab, *SettingsFrame;

  /////////////////
  // Settings frame

  TGTextButton *SetSettingsFileName_TB;
  ADAQTextEntryWithLabel *SettingsFileName_TEL;
  TGTextButton *SaveSettingsToFile_TB, *LoadSettingsFromFile_TB;
  TGCheckButton *AutoSaveSettings_CB, *AutoLoadSettings_CB;

  /////////////////////////
  // VME connection widgets

  TGTextButton *VMEConnect_TB;
  TGTextView *ConnectionOutput_TV;
  vector<ADAQComboBoxWithLabel *> BoardType_CBL;
  vector<ADAQNumberEntryFieldWithLabel *> BoardAddress_NEF;
  vector<ADAQNumberEntryWithLabel *> BoardLinkNumber_NEL;
  vector<TGTextButton *> BoardEnable_TB;
  TGTextButton *DGCalibrateADCs_TB;
  
  /////////////////
  // Register frame
  
  vector<TGNumberEntryField *> ReadAddress_NEF, ReadValueHex_NEF, ReadValueDec_NEF;
  vector<TGTextEntry *> ReadValueBinary_TE;
  vector<TGNumberEntryField *> WriteAddress_NEF, WriteValue_NEF;
  vector<TGTextButton *> Read_TB, Write_TB;
  
  ///////////////
  // Pulser frame 

  ADAQComboBoxWithLabel *V1718PulserTimeUnit_CBL[2];
  ADAQNumberEntryWithLabel *V1718PulserPeriod_NEL[2], *V1718PulserWidth_NEL[2];
  ADAQNumberEntryWithLabel *V1718PulserPulses_NEL[2];
  ADAQComboBoxWithLabel *V1718PulserStartSource_CBL[2], *V1718PulserStopSource_CBL[2];

  ADAQComboBoxWithLabel *V1718PulserOutputLine_CBL[2], *V1718PulserOutputPolarity_CBL[2];
  ADAQComboBoxWithLabel *V1718PulserLEDPolarity_CBL[2], *V1718PulserSource_CBL[2];
  TGTextButton *V1718PulserStartStop_TB[2];


  /////////////////////////////
  // High voltage frame widgets

  ADAQNumberEntryWithLabel *HVChVoltage_NEL[6], *HVChCurrent_NEL[6], *HVChRampRate_NEL[6];
  ADAQNumberEntryFieldWithLabel *HVChVoltageMonitor_NEFL[6], *HVChCurrentMonitor_NEFL[6];
  TGTextButton *HVChPower_TB[6];

  TGCheckButton *HVMonitorEnable_CB;

  //////////////////////
  // Scope frame widgets

  // Channel-specific widgets (in the left columnar subframe)
  
  // Firmware-agnostic widgets
  TGCheckButton *DGChannelLockToZero_CB;
  ADAQNumberEntryWithLabel *DGChannelLockLower_NEL, *DGChannelLockUpper_NEL;
  TGCheckButton *DGChEnable_CB[MAX_DG_CHANNELS];
  TGRadioButton *DGChPosPolarity_RB[MAX_DG_CHANNELS];
  TGRadioButton *DGChNegPolarity_RB[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChDCOffset_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChTriggerThreshold_NEL[MAX_DG_CHANNELS];
  
  // CAEN Standard firmware widgets
  
  ADAQNumberEntryWithLabel *DGChZLEThreshold_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChZLESamples_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChZLEForward_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChZLEBackward_NEL[MAX_DG_CHANNELS];
  TGRadioButton *DGChZLEPosLogic_RB[MAX_DG_CHANNELS], *DGChZLENegLogic_RB[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChBaselineCalcMin_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChBaselineCalcMax_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPSDTotalStart_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPSDTotalStop_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPSDTailStart_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPSDTailStop_NEL[MAX_DG_CHANNELS];

  // CAEN DPP-PSD firmware widgets

  ADAQNumberEntryWithLabel *DGChRecordLength_NEL[MAX_DG_CHANNELS];
  ADAQComboBoxWithLabel *DGChBaselineSamples_CBL[MAX_DG_CHANNELS];
  ADAQComboBoxWithLabel *DGChChargeSensitivity_CBL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPSDCut_NEL[MAX_DG_CHANNELS];  
  ADAQComboBoxWithLabel *DGChTriggerConfig_CBL[MAX_DG_CHANNELS];
  //ADAQNumberEntryWithLabel *DGChTriggerHoldoff_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChTriggerValidation_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChShortGate_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChLongGate_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChPreTrigger_NEL[MAX_DG_CHANNELS];
  ADAQNumberEntryWithLabel *DGChGateOffset_NEL[MAX_DG_CHANNELS];
  
  // Display specific widgets (in the upper-right subframe)

  TRootEmbeddedCanvas *DisplayCanvas_EC;
  TGDoubleVSlider *DisplayVerticalScale_DVS;
  TGTripleHSlider *DisplayHorizontalScale_THS;
  TGTextButton *AQStartStop_TB, *AQTrigger_TB, *DisplayUpdate_TB;

  // Subtab specific widgets (in the bottom-right subframe)
  
  // Data acquisition subtab
  
  TGRadioButton *AQWaveform_RB, *AQSpectrum_RB, *AQPSDHistogram_RB, *AQRate_RB;

  ADAQComboBoxWithLabel *DGTriggerType_CBL;
  ADAQComboBoxWithLabel *DGTriggerEdge_CBL;
  ADAQNumberEntryWithLabel *DGPSDTriggerHoldoff_NEL;
  TGCheckButton *DGTriggerCoincidenceEnable_CB;
  
  ADAQComboBoxWithLabel *DGAcquisitionControl_CBL;
  ADAQNumberEntryWithLabel *DGRecordLength_NEL;
  ADAQNumberEntryWithLabel *DGPostTrigger_NEL;
  ADAQComboBoxWithLabel *DGPSDMode_CBL;
  TGRadioButton *DGPSDListAnalysis_RB, *DGPSDWaveformAnalysis_RB;
  ADAQNumberEntryWithLabel *AQTime_NEL;  
  ADAQNumberEntryFieldWithLabel *AQTimer_NEFL;
  TGTextButton *AQTimerStart_TB, *AQTimerAbort_TB;

  ADAQNumberEntryWithLabel *DGEventsBeforeReadout_NEL;
  TGTextButton *DGCheckBufferStatus_TB;
  TGHProgressBar *DGBufferStatus_PB;

  TGCheckButton *AQDataReductionEnable_CB;
  ADAQNumberEntryWithLabel *AQDataReductionFactor_NEL;
  TGCheckButton *DGZLEEnable_CB;
  
  // Pulse spectra subtab

  ADAQComboBoxWithLabel *SpectrumChannel_CBL;
  ADAQNumberEntryWithLabel *SpectrumNumBins_NEL;
  ADAQNumberEntryWithLabel *SpectrumMinBin_NEL;
  ADAQNumberEntryWithLabel *SpectrumMaxBin_NEL;

  TGRadioButton *SpectrumPulseHeight_RB;
  TGRadioButton *SpectrumPulseArea_RB;
  TGCheckButton *SpectrumLDEnable_CB;
  ADAQNumberEntryWithLabel *SpectrumLLD_NEL;
  ADAQNumberEntryWithLabel *SpectrumULD_NEL;
  TGCheckButton *SpectrumLDTrigger_CB;
  ADAQComboBoxWithLabel *SpectrumLDTriggerChannel_CBL;

  TGCheckButton *SpectrumCalibration_CB;
  TGCheckButton *SpectrumUseCalibrationSlider_CB;
  ADAQComboBoxWithLabel *SpectrumCalibrationPoint_CBL;
  ADAQNumberEntryWithLabel *SpectrumCalibrationEnergy_NEL;
  ADAQNumberEntryWithLabel *SpectrumCalibrationPulseUnit_NEL;
  ADAQComboBoxWithLabel *SpectrumCalibrationUnit_CBL;
  TGTextButton *SpectrumCalibrationSetPoint_TB;
  TGTextButton *SpectrumCalibrationCalibrate_TB;
  TGTextButton *SpectrumCalibrationPlot_TB;
  TGTextButton *SpectrumCalibrationReset_TB;
  TGTextButton *SpectrumCalibrationLoad_TB;
  TGTextButton *SpectrumCalibrationWrite_TB;

  // Pulse discrimination subtab
  
  ADAQComboBoxWithLabel *PSDChannel_CBL;
  TGRadioButton *PSDYAxisTail_RB, *PSDYAxisTailTotal_RB;
  ADAQNumberEntryWithLabel *PSDThreshold_NEL;
  ADAQNumberEntryWithLabel *PSDTotalBins_NEL, *PSDTailBins_NEL;
  ADAQNumberEntryWithLabel *PSDTotalMinBin_NEL, *PSDTotalMaxBin_NEL;
  ADAQNumberEntryWithLabel *PSDTailMinBin_NEL, *PSDTailMaxBin_NEL;

  // Persistent storage subtab

  TGTextButton *WaveformFileName_TB;
  ADAQTextEntryWithLabel *WaveformFileName_TEL;
  TGTextButton *WaveformCreateFile_TB;
  TGTextButton *WaveformCloseFile_TB;
  TGTextButton *WaveformCommentFile_TB;
  TGCheckButton *WaveformStorageEnable_CB;
  TGCheckButton *WaveformStoreRaw_CB;
  TGCheckButton *WaveformStoreEnergyData_CB;
  TGCheckButton *WaveformStorePSDData_CB;

  TGRadioButton *WaveformOutput_RB, *SpectrumOutput_RB, *PSDHistogramOutput_RB;
  ADAQComboBoxWithLabel *ObjectOutputChannel_CBL;
  TGTextButton *ObjectOutputFileName_TB;
  ADAQTextEntryWithLabel *ObjectOutputFileName_TEL;
  TGCheckButton *ObjectSaveWithTimeExtension_CB;
  TGTextButton *ObjectSave_TB;
  
  TGTextButton *CanvasFileName_TB;
  ADAQTextEntryWithLabel *CanvasFileName_TEL;
  TGCheckButton *CanvasSaveWithTimeExtension_CB;
  TGTextButton *CanvasSave_TB;
  
  // Graphics
  TGCheckButton *DisplayTrigger_CB, *DisplayPSDLimits_CB;  
  TGCheckButton *DisplayBaselineBox_CB, *DisplayZLEThreshold_CB;
  TGCheckButton *DisplayLegend_CB, *DisplayGrid_CB;
  TGCheckButton *DisplayXAxisLog_CB, *DisplayYAxisLog_CB;
  
  TGRadioButton *DrawWaveformWithLine_RB, *DrawWaveformWithMarkers_RB, *DrawWaveformWithBoth_RB;
  TGRadioButton *DrawSpectrumWithLine_RB, *DrawSpectrumWithMarkers_RB, *DrawSpectrumWithBars_RB;
  
  TGCheckButton *DisplayTitlesEnable_CB;
  ADAQTextEntryWithLabel *DisplayTitle_TEL, *DisplayXTitle_TEL, *DisplayYTitle_TEL;
  ADAQNumberEntryWithLabel *DisplayXTitleOffset_NEL, *DisplayXTitleSize_NEL;
  ADAQNumberEntryWithLabel *DisplayYTitleOffset_NEL, *DisplayYTitleSize_NEL;

  ADAQNumberEntryWithLabel *SpectrumRefreshRate_NEL;

  ADAQComboBoxWithLabel *RateChannel_CBL;
  ADAQNumberEntryWithLabel *RatePlotDisp_NEL;
  ADAQNumberEntryWithLabel *RatePlotPeriod_NEL;

  TGRadioButton *DisplayContinuous_RB, *DisplayUpdateable_RB, *DisplayNonUpdateable_RB;

  // Coincidence
  ADAQNumberEntryWithLabel *DGTriggerCoincidenceWindow_NEL;
  ADAQComboBoxWithLabel *DGTriggerCoincidenceLevel_CBL;
  ADAQComboBoxWithLabel *DGTriggerCoincidenceChannel1_CBL;
  ADAQComboBoxWithLabel *DGTriggerCoincidenceChannel2_CBL;


  // Define the AAInterface class to ROOT 
  ClassDef(AAInterface, 1);
};


#endif
