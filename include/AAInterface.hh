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

  // Variables for use with digitizer control
  vector<string> DGChannelLabels;
  vector<int> DGChEnable_CB_ID_Vec;
  vector<int> DGChDCOffset_NEL_ID_Vec;
  vector<int> DGChTriggerThreshold_NEL_ID_Vec;
  vector<int> DGChBaselineCalcMin_NEL_ID_Vec;
  vector<int> DGChBaselineCalcMax_NEL_ID_Vec;
  map<int,int> DGChTriggerThreshold_NEL_ID_Map;
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
  vector<TGNumberEntryField *> BoardAddress_NEF;
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
  ADAQNumberEntryWithLabel *DGChVerticalPosition_NEL[8];
  ADAQNumberEntryWithLabel *DGChTriggerThreshold_NEL[8];
  ADAQNumberEntryWithLabel *DGChBaselineCalcMin_NEL[8];
  ADAQNumberEntryWithLabel *DGChBaselineCalcMax_NEL[8];
  ADAQNumberEntryWithLabel *DGChZSThreshold_NEL[8];
  ADAQNumberEntryWithLabel *DGChZSSamples_NEL[8];
  ADAQNumberEntryWithLabel *DGChZSForward_NEL[8];
  ADAQNumberEntryWithLabel *DGChZSBackward_NEL[8];
  TGRadioButton *DGChZSPosLogic_RB[8], *DGChZSNegLogic_RB[8];
  
  // Display specific widgets (in the upper-right subframe)

  TRootEmbeddedCanvas *DisplayCanvas_EC;
  TGDoubleVSlider *DisplayVerticalScale_DVS;
  TGTripleHSlider *DisplayHorizontalScale_THS;
  TGTextButton *AQStartStop_TB, *AQTrigger_TB, *DisplayUpdate_TB;

  // Subtab specific widgets (in the bottom-right subframe)
  
  // Acquisition subtab

  TGRadioButton *AQWaveform_RB, *AQSpectrum_RB;
  TGRadioButton *AQHighRate_RB, *AQUltraRate_RB;

  ADAQComboBoxWithLabel *DGTriggerType_CBL;
  ADAQComboBoxWithLabel *DGTriggerEdge_CBL;
  TGCheckButton *DGTriggerCoincidenceEnable_CB;
  ADAQComboBoxWithLabel *DGTriggerCoincidenceLevel_CBL;

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
  TGCheckButton *DGZSEnable_CB;
  
  // Spectrum

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

  // Graphics

  TGCheckButton *DisplayLegend_CB;
  TGCheckButton *DisplayGrid_CB;
  TGCheckButton *DisplayXAxisLog_CB;
  TGCheckButton *DisplayYAxisLog_CB;
  
  TGCheckButton *DisplayTitlesEnable_CB;
  ADAQTextEntryWithLabel *DisplayTitle_TEL, *DisplayXTitle_TEL, *DisplayYTitle_TEL;
  ADAQNumberEntryWithLabel *DisplayXTitleOffset_NEL, *DisplayXTitleSize_NEL;
  ADAQNumberEntryWithLabel *DisplayYTitleOffset_NEL, *DisplayYTitleSize_NEL;

  ADAQNumberEntryWithLabel *SpectrumRefreshRate_NEL;

  // Persistent storage

  TGTextButton *WaveformFileName_TB;
  ADAQTextEntryWithLabel *WaveformFileName_TEL;
  TGTextButton *WaveformCreateFile_TB;
  TGTextButton *WaveformCloseFile_TB;
  TGCheckButton *WaveformStorageEnable_CB;

  TGTextButton *SpectrumFileName_TB;
  ADAQTextEntryWithLabel *SpectrumFileName_TEL;
  TGCheckButton *SpectrumSaveWithTimeExtension_CB;
  TGTextButton *SpectrumSave_TB;
  
  TGTextButton *CanvasFileName_TB;
  ADAQTextEntryWithLabel *CanvasFileName_TEL;
  TGCheckButton *CanvasSaveWithTimeExtension_CB;
  TGTextButton *CanvasSave_TB;

  // Define the AAInterface class to ROOT 
  ClassDef(AAInterface, 0);
};


#endif
