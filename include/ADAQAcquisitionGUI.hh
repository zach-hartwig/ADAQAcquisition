#ifndef __ADAQAcquisitionGUI_HH__
#define __ADAQAcquisitionGUI_HH__ 1

#include <TGLabel.h>
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
#include <TGraph.h>
#include <TLegend.h>
#include <TTimer.h>
#include <TGMenu.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TH1F.h>
#include <TROOT.h>
#include <TLine.h>
#include <TGDoubleSlider.h>
#include <TGTripleSlider.h>
#include <TStyle.h>
#include <TTree.h>

#include <vector>
#include <map>
#include <string>
using namespace std;

#include "ADAQRootGUIClasses.hh"
class ADAQHighVoltage;
class ADAQDigitizer;

struct ADAQChannelCalibrationData{
  vector<int> PointID;
  vector<double> Energy;
  vector<double> PulseUnit;
};
 

class ADAQAcquisitionGUI : public TGMainFrame
{
public:

  ADAQAcquisitionGUI(int Width, int Height);
  ~ADAQAcquisitionGUI();

  // Member functions to create and fill GUI frames with widgets ("signals")
  void CreateTopLevelFrames();
  void FillConnectionFrame();
  void FillVoltageFrame();
  void FillScopeFrame();

  // Member functions for performing widget actions ("slots")
  void HandleVoltageButtons();
  void HandleConnectionButtons();
  void HandleScopeButtons();
  void HandleScopeNumberEntries();
  void HandleDisconnectAndTerminate(bool = true);

  // Member functions for real-time actions
  void RunHVMonitoring();
  void RunDGScope();

  // Member functions to control various widget states
  void SetHVWidgetState(int, bool);
  void SetDGWidgetState(bool);


private:

  ////////////////////////////////
  // General objects and variables
  ADAQHighVoltage *HVManager;
  ADAQDigitizer *DGManager;

  bool V1720Enable;
  int V1720BoardAddress;

  bool V6534Enable;
  int V6534BoardAddress;

  bool VMEConnectionEstablished;

  bool HVMonitorEnable;
  vector<string> HVChannelLabels;
  vector<int> HVChannelPower_TB_ID_Vec;
  map<int,int> HVChannelPower_TB_ID_Map;

  bool DGScopeEnable;
  vector<string> DGChannelLabels;
  vector<int> DGScopeChEnable_CB_ID_Vec;
  vector<int> DGScopeChDCOffset_NEL_ID_Vec;
  vector<int> DGScopeChTriggerThreshold_NEL_ID_Vec;
  vector<int> DGScopeChBaselineCalcMin_NEL_ID_Vec;
  vector<int> DGScopeChBaselineCalcMax_NEL_ID_Vec;

  map<int,int> DGScopeChTriggerThreshold_NEL_ID_Map;

  TColor *ColorManager;
  
  int DisplayWidth, DisplayHeight;

  TGVerticalFrame *TopFrame;

  ////////////
  // Tab frame
  TGTab *TopLevelTabs;

  ///////////////////////////////
  // VME connection frame widgets
  TGCompositeFrame *ConnectionTab, *ConnectionFrame;

  TGTextButton *V1718Connect_TB;

  TGTextButton *V6534BoardEnable_TB;
  TGNumberEntryField *V1720BoardAddress_NEF, *V1720ReadAddress_NEF, *V1720ReadValue_NEF;
  TGNumberEntryField *V1720WriteAddress_NEF, *V1720WriteValue_NEF;
  TGTextButton *V1720Read_TB, *V1720Write_TB;
  
  TGTextButton *V1720BoardEnable_TB;
  TGNumberEntryField *V6534BoardAddress_NEF, *V6534ReadAddress_NEF, *V6534ReadValue_NEF;
  TGNumberEntryField *V6534WriteAddress_NEF, *V6534WriteValue_NEF;
  TGTextButton *V6534Read_TB, *V6534Write_TB;

  /////////////////////////////
  // High voltage frame widgets
  TGCompositeFrame *VoltageTab, *VoltageFrame;

  ADAQNumberEntryWithLabel *HVChannelV_NEL[6], *HVChannelI_NEL[6];
  ADAQNumberEntryFieldWithLabel *HVChannelVMonitor_NEFL[6], *HVChannelIMonitor_NEFL[6];
  TGTextButton *HVChannelPower_TB[6];

  TGCheckButton *HVMonitorEnable_CB;

  //////////////////////
  // Scope frame widgets
  TGCompositeFrame *ScopeTab, *ScopeFrame;

  TGCheckButton *DGScopeChannelEnable_CB[8];
  TGRadioButton *DGScopeChannelPosPolarity_RB[8];
  TGRadioButton *DGScopeChannelNegPolarity_RB[8];
  ADAQNumberEntryWithLabel *DGScopeDCOffset_NEL[8];
  ADAQNumberEntryWithLabel *DGScopeVerticalPosition_NEL[8];
  ADAQNumberEntryWithLabel *DGScopeChTriggerThreshold_NEL[8];
  ADAQNumberEntryWithLabel *DGScopeBaselineCalcMin_NEL[8];
  ADAQNumberEntryWithLabel *DGScopeBaselineCalcMax_NEL[8];
  
  TRootEmbeddedCanvas *DGScope_EC;
  TGDoubleVSlider *DGScopeVerticalScale_DVS;
  TGTripleHSlider *DGScopeHorizontalScale_THS;
  TGTextButton *DGScopeStartStop_TB, *DGScopeTrigger_TB, *DGScopeSave_TB;

  ADAQComboBoxWithLabel *DGScopeTriggerMode_CBL;

  ADAQNumberEntryWithLabel *DGScopeRecordLength_NEL;
  ADAQNumberEntryWithLabel *DGScopePostTriggerSize_NEL;

  TGCheckButton *DGScopeTriggerCoincidenceEnable_CB;
  ADAQComboBoxWithLabel *DGScopeTriggerCoincidenceLevel_CBL;

  TGRadioButton *DGScopeSpectrumAnalysisHeight_RB;
  TGRadioButton *DGScopeSpectrumAnalysisArea_RB;

  ADAQNumberEntryWithLabel *DGScopeSpectrumAnalysisLLD_NEL;
  ADAQNumberEntryWithLabel *DGScopeSpectrumAnalysisULD_NEL;
  TGCheckButton *DGScopeSpectrumAnalysisLDTrigger_CB;
  ADAQComboBoxWithLabel *DGScopeSpectrumAnalysisLDTriggerChannel_CBL;

  ADAQComboBoxWithLabel *DGScopeSpectrumCalibrationPoint_CBL;
  ADAQNumberEntryWithLabel *DGScopeSpectrumCalibrationEnergy_NEL;
  ADAQNumberEntryWithLabel *DGScopeSpectrumCalibrationPulseUnit_NEL;
  TGTextButton *DGScopeSpectrumCalibrationSetPoint_TB;
  TGTextButton *DGScopeSpectrumCalibrationCalibrate_TB;
  TGTextButton *DGScopeSpectrumCalibrationPlot_TB;
  TGTextButton *DGScopeSpectrumCalibrationReset_TB;

  ADAQNumberEntryWithLabel *DGScopeSpectrumBinNumber_NEL;
  ADAQNumberEntryWithLabel *DGScopeSpectrumMinBin_NEL;
  ADAQNumberEntryWithLabel *DGScopeSpectrumMaxBin_NEL;
  TGCheckButton *DGScopeSpectrumCalibration_CB;
  TGCheckButton *DGScopeSpectrumXAxisLog_CB;
  TGCheckButton *DGScopeSpectrumYAxisLog_CB;
  TGCheckButton *DGScopeSpectrumAggregateRuns_CB;

  ADAQComboBoxWithLabel*DGScopeSpectrumChannel_CBL;

  ADAQTextEntryWithLabel *DGScopeDisplayXTitle_TEL, *DGScopeDisplayYTitle_TEL, *DGScopeDisplayTitle_TEL;
  ADAQNumberEntryWithLabel *DGScopeDisplayXTitleOffset_NEL, *DGScopeDisplayXTitleSize_NEL;
  ADAQNumberEntryWithLabel *DGScopeDisplayYTitleOffset_NEL, *DGScopeDisplayYTitleSize_NEL;

  TGCheckButton *DGScopeDisplayDrawLegend_CB;
  TGRadioButton *DGScopeDisplayWaveformXAxisSample_RB;
  TGRadioButton *DGScopeDisplayWaveformXAxisNanoseconds_RB;
  TGRadioButton *DGScopeDisplayWaveformYAxisADC_RB;
  TGRadioButton *DGScopeDisplayWaveformYAxisMillivolts_RB;

  ADAQTextEntryWithLabel *DGScopeDisplayOutputFileName_TEL;

  ADAQComboBoxWithLabel *DGScopeDisplayOutputFileType_CBL;

  TGButtonGroup *DGScopeMode_BG;
  TGRadioButton *DGScopeWaveform_RB, *DGScopeSpectrum_RB, *DGScopeBlank_RB;

  ADAQNumberEntryWithLabel *DGScopeBaselineMin_NEL, *DGScopeBaselineMax_NEL;

  ADAQTextEntryWithLabel *DGScopeDataFileName_TEL;
  ADAQTextEntryWithLabel *DGScopeDataComment_TEL;
  TGTextButton *DGScopeDataStorageCreateFile_TB;
  TGTextButton *DGScopeDataStorageCloseFile_TB;
  TGCheckButton *DGScopeDataStorageEnable_CB;

  ADAQNumberEntryWithLabel *DGScopeSpectrumRefreshRate_NEL;

  TFile *OutputDataFile;
  TTree *WaveformTree;
  bool BranchWaveformTree;
  ADAQRootMeasParams *MeasParams;
  TObjString *MeasComment;

  vector<bool> UseCalibrationManager;
  vector<TGraph *> CalibrationManager;
  vector<ADAQChannelCalibrationData> CalibrationData;

  /////////////////////
  // Quit frame widgets 
  TGHorizontalFrame *QuitFrame;
  TGTextButton *QuitButton_TB;

  //////////////////
  // DGScope objects
  TGraph *DGScopeWaveform_G[8];
  TLegend *DGScopeWaveform_L;
  TLine *DGScopeChTrigger_Line[8];

  TH1F *DGScopeSpectrum_H[8];

  // Define the ADAQAcquisitionGUI class to ROOT 
  ClassDef(ADAQAcquisitionGUI,1);
};


#endif
