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


class AAInterface : public TGMainFrame
{
  friend class AAChannelSlots;
  friend class AADisplaySlots;
  friend class AASubtabSlots;
  friend class AATabSlots;

public:

  AAInterface();
  ~AAInterface();
  
  // Create the GUI and populate with widgets
  void CreateTopLevelFrames();
  void FillConnectionFrame();
  void FillRegisterFrame();
  void FillPulserFrame();
  void FillVoltageFrame();
  void FillAcquisitionFrame();
  
  void UpdateAQTimer(int);
  void UpdateAfterAQTimerStopped(bool);
  void UpdateAfterCalibrationPointAdded(int);
  void UpdateHVMonitors(int, int, int);

  string CreateFileDialog(const char *[], EFileDialogMode);

  void HandleDisconnectAndTerminate(bool = true);

  void SaveSettings();

  // Enable/disable widgets
  void SetVoltageChannelWidgetState(int, bool);
  void SetVoltageWidgetState(bool, EButtonState);
  void SetAcquisitionWidgetState(bool, EButtonState);
  void SetCalibrationWidgetState(bool, EButtonState);
  void SetTitlesWidgetState(bool, EButtonState);

private:

  ///////////////////////////
  // General use variables //
  ///////////////////////////

  // Dimensions for interface window
  int DisplayWidth, DisplayHeight;
  int ButtonForeColor, ButtonBackColorOn, ButtonBackColorOff;
  
  // Variables for use with high voltage widgets
  vector<string> HVChLabels;
  vector<int> HVChPower_TB_ID_Vec;
  map<int,int> HVChPower_TB_ID_Map;

  // Channel-specific variables for acquisition setting widgets
  vector<string> DGChannelLabels;
  vector<int> DGChEnable_CB_ID_Vec;
  vector<int> DGChDCOffset_NEL_ID_Vec;
  vector<int> DGChTriggerThreshold_NEL_ID_Vec;
  map<int,int> DGChTriggerThreshold_NEL_ID_Map;

  // Channel-specific variables for analysis setting widgets
  vector<int> DGChBaselineCalcMin_NEL_ID_Vec;
  vector<int> DGChBaselineCalcMax_NEL_ID_Vec;
  vector<int> DGChPSDTotalStart_NEL_ID_Vec;
  vector<int> DGChPSDTotalStop_NEL_ID_Vec;
  vector<int> DGChPSDTailStart_NEL_ID_Vec;
  vector<int> DGChPSDTailStop_NEL_ID_Vec;

  const int NumDataChannels;

  // Object to convert numeric color to pixel color
  TColor *ColorManager;

    // IDs for each module
  enum{V1718, V1720, V6534};
  const int NumVMEBoards;

  AAVMEManager *TheVMEManager;
  AAAcquisitionManager *TheACQManager;

  AASettings *TheSettings;

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


  /////////////////////////
  // VME connection widgets

  TGTextButton *VMEConnect_TB;
  TGTextView *ConnectionOutput_TV;
  vector<ADAQComboBoxWithLabel *> BoardType_CBL;
  vector<ADAQNumberEntryFieldWithLabel *> BoardAddress_NEF;
  vector<ADAQNumberEntryWithLabel *> BoardLinkNumber_NEL;
  vector<TGTextButton *> BoardEnable_TB;


  /////////////////
  // Register frame

  vector<TGNumberEntryField *> ReadAddress_NEF, ReadValueHex_NEF;
  vector<TGTextEntry *> ReadValueBinary_TE;
  vector<TGNumberEntryField *> WriteAddress_NEF, WriteValue_NEF;
  vector<TGTextButton *> Read_TB, Write_TB;

  TGTextButton *HVBoardEnable_TB;
  TGNumberEntryField *HVBoardAddress_NEF, *HVReadAddress_NEF, *HVReadValue_NEF;
  TGNumberEntryField *HVWriteAddress_NEF, *HVWriteValue_NEF;
  TGTextButton *HVRead_TB, *HVWrite_TB;

  TGTextButton *DGBoardEnable_TB;
  TGNumberEntryField *DGBoardAddress_NEF, *DGReadAddress_NEF, *DGReadValue_NEF;
  TGNumberEntryField *DGWriteAddress_NEF, *DGWriteValue_NEF;
  TGTextButton *DGRead_TB, *DGWrite_TB;

  
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

  ADAQNumberEntryWithLabel *HVChVoltage_NEL[6], *HVChCurrent_NEL[6];
  ADAQNumberEntryFieldWithLabel *HVChVoltageMonitor_NEFL[6], *HVChCurrentMonitor_NEFL[6];
  TGTextButton *HVChPower_TB[6];

  TGCheckButton *HVMonitorEnable_CB;

  //////////////////////
  // Scope frame widgets

  // Channel-specific widgets (in the left columnar subframe)

  TGCheckButton *DGChEnable_CB[8];
  TGRadioButton *DGChPosPolarity_RB[8];
  TGRadioButton *DGChNegPolarity_RB[8];
  ADAQNumberEntryWithLabel *DGChDCOffset_NEL[8];
  ADAQNumberEntryWithLabel *DGChTriggerThreshold_NEL[8];
  ADAQNumberEntryWithLabel *DGChZLEThreshold_NEL[8];
  ADAQNumberEntryWithLabel *DGChZLESamples_NEL[8];
  ADAQNumberEntryWithLabel *DGChZLEForward_NEL[8];
  ADAQNumberEntryWithLabel *DGChZLEBackward_NEL[8];
  TGRadioButton *DGChZLEPosLogic_RB[8], *DGChZLENegLogic_RB[8];
  
  ADAQNumberEntryWithLabel *DGChBaselineCalcMin_NEL[8];
  ADAQNumberEntryWithLabel *DGChBaselineCalcMax_NEL[8];
  ADAQNumberEntryWithLabel *DGChPSDTotalStart_NEL[8];
  ADAQNumberEntryWithLabel *DGChPSDTotalStop_NEL[8];
  ADAQNumberEntryWithLabel *DGChPSDTailStart_NEL[8];
  ADAQNumberEntryWithLabel *DGChPSDTailStop_NEL[8];
  
  // Display specific widgets (in the upper-right subframe)

  TRootEmbeddedCanvas *DisplayCanvas_EC;
  TGDoubleVSlider *DisplayVerticalScale_DVS;
  TGTripleHSlider *DisplayHorizontalScale_THS;
  TGTextButton *AQStartStop_TB, *AQTrigger_TB, *DisplayUpdate_TB;

  // Subtab specific widgets (in the bottom-right subframe)
  
  // Data acquisition subtab
  
  TGRadioButton *AQWaveform_RB, *AQSpectrum_RB, *AQPSDHistogram_RB;

  ADAQComboBoxWithLabel *DGTriggerType_CBL;
  ADAQComboBoxWithLabel *DGTriggerEdge_CBL;
  TGCheckButton *DGTriggerCoincidenceEnable_CB;
  ADAQComboBoxWithLabel *DGTriggerCoincidenceLevel_CBL;
  
  ADAQComboBoxWithLabel *DGAcquisitionControl_CBL;
  ADAQNumberEntryWithLabel *DGRecordLength_NEL;
  ADAQNumberEntryWithLabel *DGPostTrigger_NEL;
  ADAQNumberEntryWithLabel *AQTime_NEL;  
  ADAQNumberEntryFieldWithLabel *AQTimer_NEFL;
  TGTextButton *AQTimerStart_TB, *AQTimerAbort_TB;

  ADAQNumberEntryWithLabel *DGEventsBeforeReadout_NEL;
  TGTextButton *DGCheckBufferStatus_TB;
  TGTextEntry *DGBufferStatus_TE;

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
  TGTextButton *SpectrumCalibrationSetPoint_TB;
  TGTextButton *SpectrumCalibrationCalibrate_TB;
  TGTextButton *SpectrumCalibrationPlot_TB;
  TGTextButton *SpectrumCalibrationReset_TB;
  TGTextButton *SpectrumCalibrationLoad_TB;
  TGTextButton *SpectrumCalibrationWrite_TB;

  // Pulse discrimination subtab
  
  ADAQComboBoxWithLabel *PSDChannel_CBL;
  TGRadioButton *PSDTotalVsTail_RB, *PSDTotalVsPSD_RB;
  ADAQNumberEntryWithLabel *PSDThreshold_NEL;
  ADAQNumberEntryWithLabel *PSDTotalBins_NEL, *PSDTailBins_NEL;
  ADAQNumberEntryWithLabel *PSDTotalMinBin_NEL, *PSDTotalMaxBin_NEL;
  ADAQNumberEntryWithLabel *PSDTailMinBin_NEL, *PSDTailMaxBin_NEL;

  // Persistent storage subtab

  TGTextButton *WaveformFileName_TB;
  ADAQTextEntryWithLabel *WaveformFileName_TEL;
  TGTextButton *WaveformCreateFile_TB;
  TGTextButton *WaveformCloseFile_TB;
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

  TGRadioButton *DisplayContinuous_RB, *DisplayUpdateable_RB, *DisplayNonUpdateable_RB;


  // Define the AAInterface class to ROOT 
  ClassDef(AAInterface, 1);
};


#endif
