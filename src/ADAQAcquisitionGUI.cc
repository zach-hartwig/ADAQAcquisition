// ROOT 
#include <TTree.h>
#include <TFile.h>
#include <TGText.h>
#include <TGTextView.h>

// C++ 
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>
#include <fstream>
#include <string.h>
#include <assert.h>
using namespace std;

// Boost
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

// CAEN 
#include "CAENDigitizer.h"

// ADAQ 
#include "ADAQAcquisitionGUI.hh"
#include "ADAQRootClasses.hh"
#include "ADAQHighVoltage.hh"
#include "ADAQDigitizer.hh"
#include "ADAQEnumerators.hh"


ADAQAcquisitionGUI::ADAQAcquisitionGUI(int W, int H)
  : TGMainFrame(gClient->GetRoot()),
    V1720Enable(true), V1720BoardAddress(0x00420000),
    V6534Enable(true), V6534BoardAddress(0x42420000),
    VMEConnectionEstablished(false),
    HVMonitorEnable(false), DGScopeEnable(false),
    ColorManager(new TColor),
    DisplayWidth(W), DisplayHeight(H)
{
  // Hierarchical cleanup of all child frames during destructor
  SetCleanup(kDeepCleanup);

  /////////////////////////////
  // Initialize ADAQ classes //
  /////////////////////////////
  // Create "managers" for the V6534 high voltage and V1720 digitizer
  // boards that will be used to provide information to this class as
  // well as full control over each of the boards. Note that
  // ADAQAcquisitionGUI can be run without either board by setting the
  // appropriate V*Enable boolean variables above
  HVManager = new ADAQHighVoltage;
  HVManager->SetVerbose(true);

  DGManager = new ADAQDigitizer;
  DGManager->SetVerbose(true);
  

  /////////////////////////////
  // Initialize HV variables //
  /////////////////////////////
  // Initialize C++ stdlib vectors and maps but using the
  // Boost::Assign functionality for its utility and concision

  // std::vector for HV channel labels
  HVChannelLabels += "Channel 0 (-)", "Channel 1 (-)", "Channel 2 (-)", "Channel 3 (+)", "Channel 4 (+)", "Channel 5 (+)";

  // std::vector to return the ROOT channel power widget ID from the HV channel number
  HVChannelPower_TB_ID_Vec += (int)HVChannel0Power_TB_ID, (int)HVChannel1Power_TB_ID, (int)HVChannel2Power_TB_ID,
    (int)HVChannel3Power_TB_ID, (int)HVChannel4Power_TB_ID, (int)HVChannel5Power_TB_ID;
  
  // std::map to return the HV channel number from the ROOT channel power widget ID
  insert(HVChannelPower_TB_ID_Map) 
    ((int)HVChannel0Power_TB_ID,0) ((int)HVChannel1Power_TB_ID,1) ((int)HVChannel2Power_TB_ID,2)
    ((int)HVChannel3Power_TB_ID,3) ((int)HVChannel4Power_TB_ID,4) ((int)HVChannel5Power_TB_ID,5);


  /////////////////////////////
  // Initialize DG variables //
  /////////////////////////////

  DGChannelLabels += "Channel 0", "Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7";

  DGScopeChEnable_CB_ID_Vec += (int)DGScopeCh0Enable_CB_ID, (int)DGScopeCh1Enable_CB_ID, (int)DGScopeCh2Enable_CB_ID, (int)DGScopeCh3Enable_CB_ID, 
    (int)DGScopeCh4Enable_CB_ID, (int)DGScopeCh5Enable_CB_ID, (int)DGScopeCh6Enable_CB_ID, (int)DGScopeCh7Enable_CB_ID;

  DGScopeChDCOffset_NEL_ID_Vec += (int)DGScopeCh0DCOffset_NEL_ID, (int)DGScopeCh1DCOffset_NEL_ID, (int)DGScopeCh2DCOffset_NEL_ID, (int)DGScopeCh3DCOffset_NEL_ID, 
    (int)DGScopeCh4DCOffset_NEL_ID, (int)DGScopeCh5DCOffset_NEL_ID, (int)DGScopeCh6DCOffset_NEL_ID, (int)DGScopeCh7DCOffset_NEL_ID;

  DGScopeChTriggerThreshold_NEL_ID_Vec += (int)DGScopeCh0TriggerThreshold_NEL_ID, (int)DGScopeCh1TriggerThreshold_NEL_ID, (int)DGScopeCh2TriggerThreshold_NEL_ID, (int)DGScopeCh3TriggerThreshold_NEL_ID, (int)DGScopeCh4TriggerThreshold_NEL_ID, (int)DGScopeCh5TriggerThreshold_NEL_ID, (int)DGScopeCh6TriggerThreshold_NEL_ID, (int)DGScopeCh7TriggerThreshold_NEL_ID;

  DGScopeChBaselineCalcMin_NEL_ID_Vec += (int)DGScopeCh0BaselineCalcMin_NEL_ID, (int)DGScopeCh1BaselineCalcMin_NEL_ID, (int)DGScopeCh2BaselineCalcMin_NEL_ID, (int)DGScopeCh3BaselineCalcMin_NEL_ID, (int)DGScopeCh4BaselineCalcMin_NEL_ID, (int)DGScopeCh5BaselineCalcMin_NEL_ID, (int)DGScopeCh6BaselineCalcMin_NEL_ID, (int)DGScopeCh7BaselineCalcMin_NEL_ID;

  DGScopeChBaselineCalcMax_NEL_ID_Vec += (int)DGScopeCh0BaselineCalcMax_NEL_ID, (int)DGScopeCh1BaselineCalcMax_NEL_ID, (int)DGScopeCh2BaselineCalcMax_NEL_ID, (int)DGScopeCh3BaselineCalcMax_NEL_ID, (int)DGScopeCh4BaselineCalcMax_NEL_ID, (int)DGScopeCh5BaselineCalcMax_NEL_ID, (int)DGScopeCh6BaselineCalcMax_NEL_ID, (int)DGScopeCh7BaselineCalcMax_NEL_ID;
  
  insert(DGScopeChTriggerThreshold_NEL_ID_Map)
    ((int)DGScopeCh0TriggerThreshold_NEL_ID,0) ((int)DGScopeCh1TriggerThreshold_NEL_ID,1) ((int)DGScopeCh2TriggerThreshold_NEL_ID,2) ((int)DGScopeCh3TriggerThreshold_NEL_ID,3)
    ((int)DGScopeCh4TriggerThreshold_NEL_ID,4) ((int)DGScopeCh5TriggerThreshold_NEL_ID,5) ((int)DGScopeCh6TriggerThreshold_NEL_ID,6) ((int)DGScopeCh7TriggerThreshold_NEL_ID,7);
  
  
  // Create string array used to assign labels for each channel in the
  // DGScopeWaveform_L ROOT legend object
  string DGScopeWaveformTitle[8] = {"Ch 0", "Ch 1", "Ch 2", "Ch 3", 
				    "Ch 4", "Ch 5", "Ch 6", "Ch 7"};
  
  // Create a ROOT legend for the waveform graph
  DGScopeWaveform_L = new TLegend(0.91, 0.5, 0.99, 0.95);
  
  // For each channel on the digitizer, create the appropriate label
  // and symbol in the ROOT legend using a dummy TGraph object to set
  // the line attributes. Also, initialize the TH1F objects
  // representing the pulse heigh spectrum for each channel
  for(int i=0; i<DGManager->GetNumChannels(); i++){
    TGraph *Dummy_G = new TGraph();
    Dummy_G->SetLineColor(i+1);
    Dummy_G->SetLineWidth(4);

    assert(i<9);

    DGScopeWaveform_L->AddEntry(Dummy_G, DGScopeWaveformTitle[i].c_str(), "L");
    DGScopeWaveform_L->SetFillColor(18);
    DGScopeWaveform_L->SetTextSize(0.04);
    
    DGScopeSpectrum_H[i] = 0;
  }

  // Create a dummy TLine object to add a single entry to thewaveform
  // graph legend representing the trigger lines
  TLine *Dummy_Line = new TLine();
  Dummy_Line->SetLineColor(4);
  Dummy_Line->SetLineStyle(2);
  Dummy_Line->SetLineWidth(4);
  DGScopeWaveform_L->AddEntry(Dummy_Line, "Trig", "L");
  
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
    UseCalibrationManager.push_back(false);

    CalibrationManager.push_back(new TGraph);
    
    ADAQChannelCalibrationData Init;
    CalibrationData.push_back(Init);
  }
  

  ////////////////////////////////////////
  // Create each of the top-level frame //
  ////////////////////////////////////////
  CreateTopLevelFrames();

  
  ///////////////////////////////////////
  // Fill each of the top-level frames //
  ///////////////////////////////////////
  FillConnectionFrame();
  FillVoltageFrame();
  FillScopeFrame();


  /////////////////////////////////////////////////
  // Init. and map the GUI windows, set defaults //
  /////////////////////////////////////////////////
  SetWindowName("AGNOSTIC Data Acquisition Command Center");
  Resize(DisplayWidth, DisplayHeight);
  MapSubwindows();
  MapWindow();
}


ADAQAcquisitionGUI::~ADAQAcquisitionGUI()
{;}


void ADAQAcquisitionGUI::CreateTopLevelFrames()
{
  TGCanvas *TopFrame_C = new TGCanvas(this, DisplayWidth, DisplayHeight);
  AddFrame(TopFrame_C, new TGLayoutHints(kLHintsCenterX));

  TopFrame = new TGVerticalFrame(TopFrame_C->GetViewPort(), DisplayWidth, DisplayHeight);
  TopFrame_C->SetContainer(TopFrame);


  ///////////////
  // Tab frame //
  ///////////////
  TGHorizontalFrame *TabFrame = new TGHorizontalFrame(TopFrame);
  TabFrame->SetBackgroundColor(ColorManager->Number2Pixel(15));

  TopLevelTabs = new TGTab(TabFrame, 800, 700);

  ConnectionTab = TopLevelTabs->AddTab("VME Connection");
  ConnectionFrame = new TGCompositeFrame(ConnectionTab, 60, 20, kVerticalFrame);
  ConnectionTab->AddFrame(ConnectionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  VoltageTab = TopLevelTabs->AddTab("High Voltage");
  VoltageFrame = new TGCompositeFrame(VoltageTab, 60, 20, kHorizontalFrame);
  VoltageTab->AddFrame(VoltageFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  ScopeTab = TopLevelTabs->AddTab("Oscilloscope");
  ScopeFrame = new TGCompositeFrame(ScopeTab, 60, 20, kHorizontalFrame);
  ScopeTab->AddFrame(ScopeFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  TabFrame->AddFrame(TopLevelTabs, new TGLayoutHints(kLHintsTop, 5,5,5,5));


  ////////////////
  // Quit frame //
  ////////////////
  /*
  QuitFrame = new TGHorizontalFrame(TopFrame);

  QuitFrame->SetBackgroundColor(ColorManager->Number2Pixel(16));

  QuitButton_TB = new TGTextButton(QuitFrame,"Exit");
  QuitButton_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleDisconnectAndTerminate(bool)");
  QuitButton_TB->Resize(150,40);
  QuitButton_TB->ChangeOptions(QuitButton_TB->GetOptions() | kFixedSize);
  
  QuitFrame->AddFrame(QuitButton_TB, new TGLayoutHints(kLHintsTop | kLHintsRight, 5,5,5,5));
  */

  ////////////////////////////////////////////
  // Add top level frames to the main frame //
  ////////////////////////////////////////////
  TopFrame->AddFrame(TabFrame, new TGLayoutHints(kLHintsTop, 5,5,5,5));
  //TopFrame->AddFrame(QuitFrame, new TGLayoutHints(kLHintsTop | kLHintsRight, 5,5,5,5));

  AddFrame(TopFrame, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
}


// The "ConnectionFrame" holds ROOT widgets that control aspects of
// connecting to the VME-USB interface, providing connection,
// initialization and read/write control to registers of V1720
// digitizer, V6534 high voltage, and V1718 USB-VME boards (planned).
void ADAQAcquisitionGUI::FillConnectionFrame()
{
  TGGroupFrame *V1718_GF = new TGGroupFrame(ConnectionFrame,"V1718 USB/VME Module", kHorizontalFrame);
  V1718_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  // ROOT text button that controls connection of ADAQAcquisitionGUI to the VME boards
  V1718_GF->AddFrame(V1718Connect_TB = new TGTextButton(V1718_GF, "Disconnected: click to connect", V1718Connect_TB_ID),
		     new TGLayoutHints(kLHintsCenterX, 5,5,5,5));
  V1718Connect_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V1718Connect_TB->Resize(500,40);
  V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  V1718Connect_TB->ChangeOptions(V1718Connect_TB->GetOptions() | kFixedSize);

  
  ////////////////////
  // V6534 Controls //
  ////////////////////
  // Controls of the V6534 board include settings the VME base (or
  // "board") address (match the address set on the potentiometers of
  // the physical V6534 board) and reading/writing to V6534 registers
  
  TGGroupFrame *V6534_GF = new TGGroupFrame(ConnectionFrame,"V6534 High Voltage Module",kHorizontalFrame);
  V6534_GF->SetTitlePos(TGGroupFrame::kCenter);

  TGVerticalFrame *V6534BoardAddress_VF = new TGVerticalFrame(V6534_GF);
  V6534BoardAddress_VF->AddFrame(new TGLabel(V6534BoardAddress_VF,"V6534 base address"),new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 0));

  TGHorizontalFrame *V6534BoardAddress_HF = new TGHorizontalFrame(V6534BoardAddress_VF);
  V6534BoardAddress_HF->AddFrame(new TGLabel(V6534BoardAddress_HF,"0x"), new TGLayoutHints(kLHintsExpandY, 5, 0, 0, 5));

  // ROOT number entry field for setting V6534 board address
  V6534BoardAddress_HF->AddFrame(V6534BoardAddress_NEF = new TGNumberEntryField(V6534BoardAddress_HF, 
										V6534BoardAddress_ID, 
										0,
										TGNumberFormat::kNESHex, 
										TGNumberFormat::kNEAPositive),
				 new TGLayoutHints(kLHintsExpandY, 5, 5, 0, 5));
  V6534BoardAddress_NEF->SetHexNumber(V6534BoardAddress);
  V6534BoardAddress_NEF->Resize(80,20);

  V6534BoardAddress_VF->AddFrame(V6534BoardAddress_HF,new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));


  // ROOT text button to enable/disable the use of the V6534 board
  V6534BoardAddress_VF->AddFrame(V6534BoardEnable_TB = new TGTextButton(V6534BoardAddress_VF, "Board enabled", V6534BoardEnable_TB_ID),
				 new TGLayoutHints(kLHintsCenterX));
  V6534BoardEnable_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V6534BoardEnable_TB->Resize(110,25);
  V6534BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
  V6534BoardEnable_TB->ChangeOptions(V6534BoardEnable_TB->GetOptions() | kFixedSize);
  
  
  V6534_GF->AddFrame(V6534BoardAddress_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,5));


  TGGroupFrame *V6534ReadCycle_GF = new TGGroupFrame(V6534_GF, "V6534 Read Cycle", kVerticalFrame);
    
  TGHorizontalFrame *V6534ReadCycleAddress_HF = new TGHorizontalFrame(V6534ReadCycle_GF);
  TGLabel *V6534ReadCycle_L1 = new TGLabel(V6534ReadCycleAddress_HF,"Offset address  0x");
  V6534ReadCycle_L1->Resize(130,20);
  V6534ReadCycle_L1->SetTextJustify(kTextRight);
  V6534ReadCycle_L1->ChangeOptions(V6534ReadCycle_L1->GetOptions() | kFixedSize);

  V6534ReadCycleAddress_HF->AddFrame(V6534ReadCycle_L1,
				     new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the V6534 register address to read from
  V6534ReadCycleAddress_HF->AddFrame(V6534ReadAddress_NEF = new TGNumberEntryField(V6534ReadCycleAddress_HF, 
										   V6534ReadAddress_ID, 
										   0, 
										   TGNumberFormat::kNESHex, 
										   TGNumberFormat::kNEAPositive),
				     new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V6534ReadAddress_NEF->Resize(80,20);

  TGHorizontalFrame *V6534ReadCycleValue_HF = new TGHorizontalFrame(V6534ReadCycle_GF);

  TGLabel *V6534ReadCycle_L2 = new TGLabel(V6534ReadCycleValue_HF,"   Value  0x");
  V6534ReadCycle_L2->Resize(130,20);
  V6534ReadCycle_L2->SetTextJustify(kTextRight);
  V6534ReadCycle_L2->ChangeOptions(V6534ReadCycle_L1->GetOptions() | kFixedSize);
    
  V6534ReadCycleValue_HF->AddFrame(V6534ReadCycle_L2,
				   new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for displaying the value from the read register address
  V6534ReadCycleValue_HF->AddFrame(V6534ReadValue_NEF = new TGNumberEntryField(V6534ReadCycleValue_HF, 
									       V6534ReadValue_ID, 
									       0, 
									       TGNumberFormat::kNESHex, 
									       TGNumberFormat::kNEAPositive),
				   new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V6534ReadValue_NEF->Resize(80,20);
  V6534ReadValue_NEF->SetState(false);

  V6534ReadCycle_GF->AddFrame(V6534ReadCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V6534ReadCycle_GF->AddFrame(V6534ReadCycleValue_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V6534ReadCycle_GF->AddFrame(V6534Read_TB = new TGTextButton(V6534ReadCycle_GF,"VME Read",V6534Read_ID),
			      new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
								   
  V6534Read_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V6534Read_TB->Resize(150,40);
  V6534Read_TB->ChangeOptions(V6534Read_TB->GetOptions() | kFixedSize);

  V6534_GF->AddFrame(V6534ReadCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));


  TGGroupFrame *V6534WriteCycle_GF = new TGGroupFrame(V6534_GF, "V6534 Write Cycle", kVerticalFrame);
  
  TGHorizontalFrame *V6534WriteCycleAddress_HF = new TGHorizontalFrame(V6534WriteCycle_GF);

  TGLabel *V6534WriteCycle_L1 = new TGLabel(V6534WriteCycleAddress_HF,"Offset Address  0x");
  V6534WriteCycle_L1->Resize(130,20);
  V6534WriteCycle_L1->SetTextJustify(kTextRight);
  V6534WriteCycle_L1->ChangeOptions(V6534WriteCycle_L1->GetOptions() | kFixedSize);

  V6534WriteCycleAddress_HF->AddFrame(V6534WriteCycle_L1,
				      new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the V6534 register address to write to
  V6534WriteCycleAddress_HF->AddFrame(V6534WriteAddress_NEF = new TGNumberEntryField(V6534WriteCycleAddress_HF, 
										     V6534WriteAddress_ID, 
										     0, 
										     TGNumberFormat::kNESHex, 
										     TGNumberFormat::kNEAPositive),
				      new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V6534WriteAddress_NEF->Resize(80,20);
  
  TGHorizontalFrame *V6534WriteCycleValue_HF = new TGHorizontalFrame(V6534WriteCycle_GF);

  TGLabel *V6534WriteCycle_L2 = new TGLabel(V6534WriteCycleValue_HF,"   Value  0x");
  V6534WriteCycle_L2->Resize(130,20);
  V6534WriteCycle_L2->SetTextJustify(kTextRight);
  V6534WriteCycle_L2->ChangeOptions(V6534WriteCycle_L2->GetOptions() | kFixedSize);

  V6534WriteCycleValue_HF->AddFrame(V6534WriteCycle_L2,
				    new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the value that will be written to the set write register address
  V6534WriteCycleValue_HF->AddFrame(V6534WriteValue_NEF = new TGNumberEntryField(V6534WriteCycleValue_HF, 
										 V6534WriteValue_ID, 
										 0, 
										 TGNumberFormat::kNESHex, 
										 TGNumberFormat::kNEAPositive),
				    new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V6534WriteValue_NEF->Resize(80,20);

  V6534WriteCycle_GF->AddFrame(V6534WriteCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V6534WriteCycle_GF->AddFrame(V6534WriteCycleValue_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V6534WriteCycle_GF->AddFrame(V6534Write_TB = new TGTextButton(V6534WriteCycle_GF,"VME Write",V6534Write_ID),
			       new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
								   
  V6534Write_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V6534Write_TB->Resize(150,40);
  V6534Write_TB->ChangeOptions(V6534Write_TB->GetOptions() | kFixedSize);

  V6534_GF->AddFrame(V6534WriteCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));


  ////////////////////
  // V1720 Controls //
  ////////////////////
  // Controls of the V1720 board include settings the VME base (or
  // "board") address (match the address set on the potentiometers of
  // the physical V1720 board) and reading/writing to V1720 registers

  TGGroupFrame *V1720_GF = new TGGroupFrame(ConnectionFrame,"V1720 Digitizer Module",kHorizontalFrame);
  V1720_GF->SetTitlePos(TGGroupFrame::kCenter);

  TGVerticalFrame *V1720BoardAddress_VF = new TGVerticalFrame(V1720_GF);
  V1720BoardAddress_VF->AddFrame(new TGLabel(V1720BoardAddress_VF,"V1720 base address"),new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 0));

  TGHorizontalFrame *V1720BoardAddress_HF = new TGHorizontalFrame(V1720BoardAddress_VF);
  V1720BoardAddress_HF->AddFrame(new TGLabel(V1720BoardAddress_HF,"0x"), new TGLayoutHints(kLHintsExpandY, 5, 0, 0, 5));
  V1720BoardAddress_HF->AddFrame(V1720BoardAddress_NEF = new TGNumberEntryField(V1720BoardAddress_HF, 
										V1720BoardAddress_ID,
										0,
										TGNumberFormat::kNESHex,
										TGNumberFormat::kNEAPositive),
				 new TGLayoutHints(kLHintsExpandY, 5, 5, 0, 5));
  V1720BoardAddress_NEF->SetHexNumber(V1720BoardAddress);
  V1720BoardAddress_NEF->Resize(80,20);
	
  V1720BoardAddress_VF->AddFrame(V1720BoardAddress_HF,new TGLayoutHints(kLHintsExpandY, 5,5,5,5));
  
  // A ROOT check button to enable/disable the use of the V1720 board
  V1720BoardAddress_VF->AddFrame(V1720BoardEnable_TB = new TGTextButton(V1720BoardAddress_VF, "Board enabled", V1720BoardEnable_TB_ID),
				 new TGLayoutHints(kLHintsCenterX));
  V1720BoardEnable_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V1720BoardEnable_TB->Resize(115,25);
  V1720BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
  V1720BoardEnable_TB->ChangeOptions(V1720BoardEnable_TB->GetOptions() | kFixedSize);

    
  V1720_GF->AddFrame(V1720BoardAddress_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,5));
			 
  TGGroupFrame *V1720ReadCycle_GF = new TGGroupFrame(V1720_GF, "V1720 Read Cycle", kVerticalFrame);
  
  TGHorizontalFrame *V1720ReadCycleAddress_HF = new TGHorizontalFrame(V1720ReadCycle_GF);
  TGLabel *V1720ReadCycle_L1 = new TGLabel(V1720ReadCycleAddress_HF,"Offset address  0x");
  V1720ReadCycle_L1->Resize(130,20);
  V1720ReadCycle_L1->SetTextJustify(kTextRight);
  V1720ReadCycle_L1->ChangeOptions(V1720ReadCycle_L1->GetOptions() | kFixedSize);

  V1720ReadCycleAddress_HF->AddFrame(V1720ReadCycle_L1,
				     new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the V1720 register address to read from
  V1720ReadCycleAddress_HF->AddFrame(V1720ReadAddress_NEF = new TGNumberEntryField(V1720ReadCycleAddress_HF, 
										   V1720ReadAddress_ID, 
										   0, 
										   TGNumberFormat::kNESHex, 
										   TGNumberFormat::kNEAPositive),
				     new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V1720ReadAddress_NEF->Resize(80,20);

  TGHorizontalFrame *V1720ReadCycleValue_HF = new TGHorizontalFrame(V1720ReadCycle_GF);

  TGLabel *V1720ReadCycle_L2 = new TGLabel(V1720ReadCycleValue_HF,"   Value  0x");
  V1720ReadCycle_L2->Resize(130,20);
  V1720ReadCycle_L2->SetTextJustify(kTextRight);
  V1720ReadCycle_L2->ChangeOptions(V1720ReadCycle_L1->GetOptions() | kFixedSize);
    
  V1720ReadCycleValue_HF->AddFrame(V1720ReadCycle_L2,
				   new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));
  
  // ROOT number entry widget for displaying the value from the read register address
  V1720ReadCycleValue_HF->AddFrame(V1720ReadValue_NEF = new TGNumberEntryField(V1720ReadCycleValue_HF, 
									       V1720ReadValue_ID, 
									       0, 
									       TGNumberFormat::kNESHex, 
									       TGNumberFormat::kNEAPositive),
				   new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V1720ReadValue_NEF->Resize(80,20);
  V1720ReadValue_NEF->SetState(false);

  V1720ReadCycle_GF->AddFrame(V1720ReadCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V1720ReadCycle_GF->AddFrame(V1720ReadCycleValue_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V1720ReadCycle_GF->AddFrame(V1720Read_TB = new TGTextButton(V1720ReadCycle_GF,"VME Read",V1720Read_ID),
			      new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
								   
  V1720Read_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V1720Read_TB->Resize(150,40);
  V1720Read_TB->ChangeOptions(V1720Read_TB->GetOptions() | kFixedSize);

  V1720_GF->AddFrame(V1720ReadCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));


  TGGroupFrame *V1720WriteCycle_GF = new TGGroupFrame(V1720_GF, "V1720 Write Cycle", kVerticalFrame);
  
  TGHorizontalFrame *V1720WriteCycleAddress_HF = new TGHorizontalFrame(V1720WriteCycle_GF);

  TGLabel *V1720WriteCycle_L1 = new TGLabel(V1720WriteCycleAddress_HF,"Offset Address  0x");
  V1720WriteCycle_L1->Resize(130,20);
  V1720WriteCycle_L1->SetTextJustify(kTextRight);
  V1720WriteCycle_L1->ChangeOptions(V1720WriteCycle_L1->GetOptions() | kFixedSize);

  V1720WriteCycleAddress_HF->AddFrame(V1720WriteCycle_L1,
				      new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the V1720 register address to write to 
  V1720WriteCycleAddress_HF->AddFrame(V1720WriteAddress_NEF = new TGNumberEntryField(V1720WriteCycleAddress_HF, 
										     V1720WriteAddress_ID, 
										     0, 
										     TGNumberFormat::kNESHex, 
										     TGNumberFormat::kNEAPositive),
				     new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V1720WriteAddress_NEF->Resize(80,20);

  TGHorizontalFrame *V1720WriteCycleValue_HF = new TGHorizontalFrame(V1720WriteCycle_GF);

  TGLabel *V1720WriteCycle_L2 = new TGLabel(V1720WriteCycleValue_HF,"   Value  0x");
  V1720WriteCycle_L2->Resize(130,20);
  V1720WriteCycle_L2->SetTextJustify(kTextRight);
  V1720WriteCycle_L2->ChangeOptions(V1720WriteCycle_L2->GetOptions() | kFixedSize);

  V1720WriteCycleValue_HF->AddFrame(V1720WriteCycle_L2,
				    new TGLayoutHints(kLHintsLeft, 5, 0, 5, 5));

  // ROOT number entry field for setting the value that will be written to the set register address
  V1720WriteCycleValue_HF->AddFrame(V1720WriteValue_NEF = new TGNumberEntryField(V1720WriteCycleValue_HF, 
										 V1720WriteValue_ID, 
										 0, 
										 TGNumberFormat::kNESHex, 
										 TGNumberFormat::kNEAPositive),
				    new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
  V1720WriteValue_NEF->Resize(80,20);
  
  V1720WriteCycle_GF->AddFrame(V1720WriteCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V1720WriteCycle_GF->AddFrame(V1720WriteCycleValue_HF, new TGLayoutHints(kLHintsExpandY, 5, 5, 5, 5));
  V1720WriteCycle_GF->AddFrame(V1720Write_TB = new TGTextButton(V1720WriteCycle_GF,"VME Write",V1720Write_ID),
			      new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
								   
  V1720Write_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleConnectionButtons()");
  V1720Write_TB->Resize(150,40);
  V1720Write_TB->ChangeOptions(V1720Write_TB->GetOptions() | kFixedSize);

  V1720_GF->AddFrame(V1720WriteCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  /////////////////////////////////////////////////
  // Add the group frames to the ConnectionFrame //
  /////////////////////////////////////////////////

  ConnectionFrame->AddFrame(V1718_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  ConnectionFrame->AddFrame(V6534_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  ConnectionFrame->AddFrame(V1720_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
}


// The "VoltageFrame" holds ROOT widgets for complete control of the
// HV supply board, including real-time monitoring of each channel's
// active voltage and drawn current. Setting the voltage and current
// for an individual channel are disabled while the channel power is
// on. This may be updated in the future to enable real-time changes.
void ADAQAcquisitionGUI::FillVoltageFrame()
{
  TGVerticalFrame *HVChannelControls_VF = new TGVerticalFrame(VoltageFrame);
  
  // The widgets and layout for each channel are identical although
  // the underlying functionality must provide for setting the
  // appropriate V6534 registers for each unique channel. Thus,
  // building and laying out the HV widgets is handled in a "for"
  // loop, with previously initialized std::vectors and std::maps used
  // to retrieve unique values for each channel where necessary. Note:
  // widget IDs are set to "-1" when the widget ID is not explicity
  // needed in the code.
  for(int ch=0; ch<HVManager->GetNumChannels(); ch++){

    TGGroupFrame *HVChannel_GF = new TGGroupFrame(HVChannelControls_VF, HVChannelLabels[ch].c_str(), kHorizontalFrame);
    HVChannel_GF->SetTitlePos(TGGroupFrame::kCenter);
    HVChannelControls_VF->AddFrame(HVChannel_GF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    
    TGVerticalFrame *HVChannelSet_VF = new TGVerticalFrame(HVChannel_GF);
    HVChannel_GF->AddFrame(HVChannelSet_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 15,15,5,0));
    
    // ADAQ number entry for setting the channel voltage
    HVChannelSet_VF->AddFrame(HVChannelV_NEL[ch] = new ADAQNumberEntryWithLabel(HVChannelSet_VF, "Set Voltage [V]",-1),
			      new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    HVChannelV_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    HVChannelV_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    HVChannelV_NEL[ch]->GetEntry()->SetLimitValues(0,HVManager->GetMaxVoltage());
    HVChannelV_NEL[ch]->GetEntry()->SetNumber(0);
    
    // ADAQ number entry for setting maximum channel current that can be drawn
    HVChannelSet_VF->AddFrame(HVChannelI_NEL[ch] = new ADAQNumberEntryWithLabel(HVChannelSet_VF, "Set Current [uA]",-1),
			      new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    HVChannelI_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    HVChannelI_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    HVChannelI_NEL[ch]->GetEntry()->SetLimitValues(0,HVManager->GetMaxCurrent());
    HVChannelI_NEL[ch]->GetEntry()->SetNumber(0);
    
    
    TGVerticalFrame *HVChannelGet_VF = new TGVerticalFrame(HVChannel_GF);
    HVChannel_GF->AddFrame(HVChannelGet_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 15,15,5,0));

    // ADAQ number entry field for displaying the real-time channel voltage [in volts]
    HVChannelGet_VF->AddFrame(HVChannelVMonitor_NEFL[ch] = new ADAQNumberEntryFieldWithLabel(HVChannelGet_VF, "Active Voltage [V]", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(false);
    
    // ADAQ number entry field for display the real-time channel current [in microamps]
    HVChannelGet_VF->AddFrame(HVChannelIMonitor_NEFL[ch] = new ADAQNumberEntryFieldWithLabel(HVChannelGet_VF, "Active Current [uA]", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(false);

    // ROOT text button to control channel power. The widget ID is set
    // using a std::vector to assign a unique ID from the ADAQ widget
    // ID enumerator (see file "ADAQEnumerators.hh") for each channel
    HVChannel_GF->AddFrame(HVChannelPower_TB[ch] = new TGTextButton(HVChannel_GF, "OFF", HVChannelPower_TB_ID_Vec[ch]),
			   new TGLayoutHints(kLHintsNormal,15,15,15,5));
    HVChannelPower_TB[ch]->Connect("Pressed()","ADAQAcquisitionGUI",this,"HandleVoltageButtons()");
    HVChannelPower_TB[ch]->SetToolTipText("Engage high voltage!");
    HVChannelPower_TB[ch]->Resize(110,50);
    HVChannelPower_TB[ch]->ChangeOptions(HVChannelPower_TB[ch]->GetOptions() | kFixedSize);
    HVChannelPower_TB[ch]->SetBackgroundColor(ColorManager->Number2Pixel(2));
    HVChannelPower_TB[ch]->SetForegroundColor(ColorManager->Number2Pixel(1));
    
    // Modify the widget background to distinguish the negative voltage 
    // (V6534 channels 0,1,2) from the positive voltage (V6534 channels 3,4,5)
    (ch<3) ? (HVChannel_GF->SetBackgroundColor(ColorManager->Number2Pixel(22))) : (HVChannel_GF->SetBackgroundColor(ColorManager->Number2Pixel(16)));
  }

  TGGroupFrame *HVAllChannel_GF = new TGGroupFrame(HVChannelControls_VF, "All Channels", kHorizontalFrame);
  HVAllChannel_GF->SetTitlePos(TGGroupFrame::kCenter);
  HVChannelControls_VF->AddFrame(HVAllChannel_GF, new TGLayoutHints(kLHintsCenterX, 5,5,5,0));

  // ROOT check button to enable/disable real-time monitoring of all
  // channel's set voltage and drawn current
  HVAllChannel_GF->AddFrame(HVMonitorEnable_CB = new TGCheckButton(HVAllChannel_GF, "Enable monitoring", HVEnableMonitoring_CB_ID),
			    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  HVMonitorEnable_CB->Connect("Clicked()", "ADAQAcquisitionGUI", this, "HandleVoltageButtons()");
  HVMonitorEnable_CB->SetState(kButtonUp);
  
  VoltageFrame->AddFrame(HVChannelControls_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
}


// The "ScopeFrame" holds ROOT widgets and displays that essentially
// function as a digital oscilloscope and multichannel analyzer
// (MCA). It has options for individual channel controls, general
// scope/MCA functionality, display options, and data storage.
void ADAQAcquisitionGUI::FillScopeFrame()
{
  //////////////////////////////
  // Fill left vertical panel //
  //////////////////////////////
  // The left-most vertical subframe on the DGScope frame is contains
  // channel-specific settings for each of the 8 digitizer
  // channels. The subframe makes use of the TGCanvas class to
  // incorporate sliders that can be used to view all channel widgets
  // in a smaller frame. Pro'n'shit if I do say so myself. And I do.

  TGCanvas *DGScopeChannelControls_C = new TGCanvas(ScopeFrame,300,100,kSunkenFrame);
  ScopeFrame->AddFrame(DGScopeChannelControls_C, new TGLayoutHints(kLHintsNormal | kLHintsExpandY));
  
  TGVerticalFrame *DGScopeChannelControls_VF = new TGVerticalFrame(DGScopeChannelControls_C->GetViewPort(),10,10);
  DGScopeChannelControls_C->SetContainer(DGScopeChannelControls_VF);

  // The widgets and layouts for control of channel-relevant
  // parameters for each of the 8 digitizers is identical although the
  // underlying functionality must correctly identify and set values
  // for each channel correctly. The widgets and layouts are therefore
  // constructed in a "for" loop previously initialized using
  // std::vectors to assign parameters that are unique to each channel
  // (names and IDs).
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){

    // Each channel's widgets are grouped under its own TGGroupFrame.
    TGGroupFrame *DGScopeChannelControl_GF = new TGGroupFrame(DGScopeChannelControls_VF, DGChannelLabels[ch].c_str(), kVerticalFrame);
    DGScopeChannelControls_VF->AddFrame(DGScopeChannelControl_GF, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 0));
    DGScopeChannelControl_GF->SetTitlePos(TGGroupFrame::kLeft);

    // Horizontal frame to hold the "enable" and "pulse polarity" buttons
    TGHorizontalFrame *DGScopeChannelControl_HF = new TGHorizontalFrame(DGScopeChannelControl_GF);
    DGScopeChannelControl_GF->AddFrame(DGScopeChannelControl_HF);
    
    // ROOT check button to enable channel for digitization
    DGScopeChannelControl_HF->AddFrame(DGScopeChannelEnable_CB[ch] = new TGCheckButton(DGScopeChannelControl_HF, "Enable", DGScopeChEnable_CB_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsCenterY,0,0,0,0));
    if(ch==0) 
      DGScopeChannelEnable_CB[ch]->SetState(kButtonDown);

    // TGLabel for the pulse polarity radio buttons
    DGScopeChannelControl_HF->AddFrame(new TGLabel(DGScopeChannelControl_HF,"Pulse \n polarity"),
				       new TGLayoutHints(kLHintsCenterY,25,0,5,0));

    // ROOT radio buttons to select the pulse polarity ("-" means
    // below the baseline; "+" means above the baseline)
    TGHButtonGroup *DGScopeChannelPolarity_BG = new TGHButtonGroup(DGScopeChannelControl_HF, "Pulse Polarity");//, kHorizontalFrame);
    DGScopeChannelPolarity_BG->SetTitlePos(TGButtonGroup::kCenter);
    DGScopeChannelPolarity_BG->SetBorderDrawn(false);
    DGScopeChannelControl_HF->AddFrame(DGScopeChannelPolarity_BG, new TGLayoutHints(kLHintsRight,-1,-15,-10,-10));

    DGScopeChannelPosPolarity_RB[ch] = new TGRadioButton(DGScopeChannelPolarity_BG, "+  ", -1);
    DGScopeChannelNegPolarity_RB[ch] = new TGRadioButton(DGScopeChannelPolarity_BG, "-", -1);
    DGScopeChannelNegPolarity_RB[ch]->SetState(kButtonDown);
    DGScopeChannelPolarity_BG->Show();


    // ADAQ number entry to set channel's vertical position on graph [ADC]
    DGScopeChannelControl_GF->AddFrame(DGScopeVerticalPosition_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Vert. Position (ADC)", DGScopeChVerticalPosition_NEL_ID),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeVerticalPosition_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeVerticalPosition_NEL[ch]->GetEntry()->SetNumber(0);
    DGScopeVerticalPosition_NEL[ch]->GetEntry()->Resize(65,20);

    // ADAQ number entry to set channel's DAC offset 
    DGScopeChannelControl_GF->AddFrame(DGScopeDCOffset_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "DC offset (hex)", DGScopeChDCOffset_NEL_ID_Vec[ch]),
					new TGLayoutHints(kLHintsNormal));
    DGScopeDCOffset_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESHex);
    DGScopeDCOffset_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    DGScopeDCOffset_NEL[ch]->GetEntry()->SetLimitValues(0x0000,0xffff);
    DGScopeDCOffset_NEL[ch]->GetEntry()->SetNumber(0x8000);
    DGScopeDCOffset_NEL[ch]->GetEntry()->Resize(65,20);

    // ADAQ number entry to set channel's trigger threshold [ADC]
    DGScopeChannelControl_GF->AddFrame(DGScopeChTriggerThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Trig. Threshold (ADC)", DGScopeChTriggerThreshold_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)","ADAQAcquisitionGUI",this,"HandleScopeNumberEntries()");
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(2000);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Resize(65,20);
    
    // ADAQ number entry to set minimum sample for baseline calculation [sample]
    DGScopeChannelControl_GF->AddFrame(DGScopeBaselineCalcMin_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Baseline min. (sample)", DGScopeChBaselineCalcMin_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)","ADAQAcquisitionGUI",this,"HandleScopeNumberEntries()");
    
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->SetNumber(10);
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->Resize(65,20);

    // ADAQ number entry to set maximum sample for baseline calculation [sample]
    DGScopeChannelControl_GF->AddFrame(DGScopeBaselineCalcMax_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Baseline max. (sample)", DGScopeChBaselineCalcMax_NEL_ID_Vec[ch]),
					   new TGLayoutHints(kLHintsNormal));
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->SetNumber(45);
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->Resize(65,20);
  }
  


  ////////////////////////////////
  // Fill DGScope display frame //
  ////////////////////////////////
  // The display frame contains the ROOT TRootEmbeddedCanvas, which
  // displays the waveforms and pulse heigh spectra, and two sliders,
  // which control the vertical and horizontal zoom of the graphs
  // along with spectrum calibration via the 3rd slider on the
  // horizontal axis.

  TGVerticalFrame *DGScopeDisplayAndControls_VF = new TGVerticalFrame(ScopeFrame);
  
  TGGroupFrame *DGScopeDisplay_GF = new TGGroupFrame(DGScopeDisplayAndControls_VF, "Digital Oscilloscope", kVerticalFrame);
  DGScopeDisplay_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  TGHorizontalFrame *DGScopeDisplayAndSlider_HF = new TGHorizontalFrame(DGScopeDisplay_GF);
  DGScopeDisplay_GF->AddFrame(DGScopeDisplayAndSlider_HF, new TGLayoutHints(kLHintsNormal));

  // ROOT double slider for control of the min/max of vertical axis, ie, zoom
  DGScopeDisplayAndSlider_HF->AddFrame(DGScopeVerticalScale_DVS = new TGDoubleVSlider(DGScopeDisplayAndSlider_HF, 400, kDoubleScaleBoth, -1, kVerticalFrame, ColorManager->Number2Pixel(17),true,false),
				       new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));
  DGScopeVerticalScale_DVS->SetRange(0,1);
  DGScopeVerticalScale_DVS->SetPosition(0,1);

  // ROOT embdedded canvas for display of waveforms and spectra
  DGScopeDisplayAndSlider_HF->AddFrame(DGScope_EC = new TRootEmbeddedCanvas("DGScope_EC", DGScopeDisplayAndSlider_HF, 650, 400),
				       new TGLayoutHints(kLHintsCenterX, 5,5,5,5));
  DGScope_EC->GetCanvas()->SetFillColor(0);
  DGScope_EC->GetCanvas()->SetFrameFillColor(0);
  DGScope_EC->GetCanvas()->SetGrid();
  DGScope_EC->GetCanvas()->SetBorderMode(0);
  DGScope_EC->GetCanvas()->SetLeftMargin(0.12);
  DGScope_EC->GetCanvas()->SetBottomMargin(0.12);
  DGScope_EC->GetCanvas()->SetTopMargin(0.08);
  DGScope_EC->GetCanvas()->SetRightMargin(0.1);

  // ROOT triple slider. The "double" slider features are used to //
  // control the min/max of the horizontal axis, ie, zoom; The "third"
  // slider is used for graphical valibration of the pulse height
  // spectrum when DGScope is set to "calibration mode" while
  // acquiring data in "spectrum mode"
  DGScopeDisplay_GF->AddFrame(DGScopeHorizontalScale_THS = new TGTripleHSlider(DGScopeDisplay_GF, 650, kDoubleScaleBoth, -1, kVerticalFrame, ColorManager->Number2Pixel(17)),
			      new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));
  DGScopeHorizontalScale_THS->SetRange(0,1);
  DGScopeHorizontalScale_THS->SetPosition(0,1);
  DGScopeHorizontalScale_THS->SetPointerPosition(0.5);
  
  TGHorizontalFrame *DGScopeDisplayControls_HF = new TGHorizontalFrame(DGScopeDisplay_GF);
  DGScopeDisplay_GF->AddFrame(DGScopeDisplayControls_HF,
			      new TGLayoutHints(kLHintsCenterX));


  ///////////////////////////////////////
  // Create large "acquisition" button //
  ///////////////////////////////////////
  // A large TGTextButton to control start/stop of the data
  // acquisition is create here, although it will not actually be
  // added to the DGScopeDisplayAndControl_VF frame until the very end
  // of this function so that we can control the order things appear

  // ROOT text button for starting/stopping data acquisition by the digitizer
  DGScopeStartStop_TB = new TGTextButton(DGScopeDisplayAndControls_VF, "Stopped", DGScopeStartStop_TB_ID);
  DGScopeStartStop_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeStartStop_TB->Resize(400,30);
  DGScopeStartStop_TB->ChangeOptions(DGScopeStartStop_TB->GetOptions() | kFixedSize);
  DGScopeStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  DGScopeStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(1));

  
  //////////////////////////////////
  // Fill horizontal bottom panel //
  //////////////////////////////////
  // The bottom frame of DGScope contains a large number of general
  // DGScope controls, grouped logically into different tabs for
  // logical accessibility. The "Scope Settings" tab contains features
  // that control operation of DGScope, such as data acquisition,
  // trigger setup, and spectrum controls. The "Display Settings"
  // controls graphical display, such as setting axis titles and
  // position. The "Data Storage" tab contains options for storing
  // acquired data in ROOT files.

  TGHorizontalFrame *DGScopeControlTabs_HF = new TGHorizontalFrame(DGScopeDisplayAndControls_VF);
  TGTab *DGScopeControlTabs = new TGTab(DGScopeControlTabs_HF);
  DGScopeControlTabs_HF->AddFrame(DGScopeControlTabs, new TGLayoutHints(kLHintsNormal,10,10,0,0));

  TGCompositeFrame *DGScopeSettingsTab = DGScopeControlTabs->AddTab("Scope Controls");
  TGCompositeFrame *DGScopeSettingsFrame = new TGCompositeFrame(DGScopeSettingsTab,0,0,kHorizontalFrame);
  DGScopeSettingsTab->AddFrame(DGScopeSettingsFrame);

  TGCompositeFrame *DGScopeSpectrumTab = DGScopeControlTabs->AddTab("Spectrum Settings");
  TGCompositeFrame *DGScopeSpectrumFrame = new TGCompositeFrame(DGScopeSpectrumTab,0,0,kHorizontalFrame);
  DGScopeSpectrumTab->AddFrame(DGScopeSpectrumFrame);

  TGCompositeFrame *DGScopeDisplaySettingsTab = DGScopeControlTabs->AddTab("Graphical Options");
  TGCompositeFrame *DGScopeDisplaySettingsFrame = new TGCompositeFrame(DGScopeDisplaySettingsTab,0,0,kHorizontalFrame);
  DGScopeDisplaySettingsTab->AddFrame(DGScopeDisplaySettingsFrame);

  TGCompositeFrame *DGScopeDataStorageTab = DGScopeControlTabs->AddTab("Data Storage");
  TGCompositeFrame *DGScopeDataStorageFrame = new TGCompositeFrame(DGScopeDataStorageTab,0,0,kHorizontalFrame);
  DGScopeDataStorageTab->AddFrame(DGScopeDataStorageFrame);

  TGCompositeFrame *DGScopeMiscTab = DGScopeControlTabs->AddTab("Miscellaneous");
  TGCompositeFrame *DGScopeMiscFrame = new TGCompositeFrame(DGScopeMiscTab,0,0,kHorizontalFrame);
  DGScopeMiscTab->AddFrame(DGScopeMiscFrame);


  ////////////////////
  // Scope settings //
  ////////////////////

  ////////////////
  // Mode controls 

  // ROOT radio buttons to specify operational mode of DGScope:
  // "Waveform" == oscilloscope, "Spectrum" == MCA, "Blank" == display
  // no graphics for high data throughput
  TGButtonGroup *DGScopeMode_BG = new TGButtonGroup(DGScopeSettingsFrame, "DGScope Mode", kVerticalFrame);
  DGScopeMode_BG->SetTitlePos(TGButtonGroup::kCenter);
  DGScopeSettingsFrame->AddFrame(DGScopeMode_BG, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  DGScopeWaveform_RB = new TGRadioButton(DGScopeMode_BG, "Waveform", DGScopeWaveform_RB_ID);
  DGScopeWaveform_RB->SetState(kButtonDown);

  DGScopeSpectrum_RB = new TGRadioButton(DGScopeMode_BG, "Spectrum", DGScopeSpectrum_RB_ID);

  DGScopeBlank_RB = new TGRadioButton(DGScopeMode_BG, "Blank", DGScopeBlank_RB_ID);
  
  DGScopeMode_BG->Show();

  ///////////////////
  // Trigger controls

  TGGroupFrame *DGScopeTriggerControls_GF = new TGGroupFrame(DGScopeSettingsFrame, "Trigger", kVerticalFrame);
  DGScopeTriggerControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSettingsFrame->AddFrame(DGScopeTriggerControls_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerCoincidenceEnable_CB = new TGCheckButton(DGScopeTriggerControls_GF, "Coincidence triggering",DGScopeTriggerCoincidenceEnable_CB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,5,0));

  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerCoincidenceLevel_CBL = new ADAQComboBoxWithLabel(DGScopeTriggerControls_GF, "Coincidence level",-1),
				     new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("2 channel",1);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("3 channel",2);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("4 channel",3);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("5 channel",4);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("6 channel",5);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("7 channel",6);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("8 channel",7);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->Select(1);

  // ADAQ combo box to enable specification of trigger type
  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerMode_CBL = new ADAQComboBoxWithLabel(DGScopeTriggerControls_GF, "Type", DGScopeTriggerMode_CBL_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeTriggerMode_CBL->GetComboBox()->AddEntry("External (NIM)",0);
  DGScopeTriggerMode_CBL->GetComboBox()->AddEntry("External (TTL)",1);
  DGScopeTriggerMode_CBL->GetComboBox()->AddEntry("Automatic",2);
  DGScopeTriggerMode_CBL->GetComboBox()->AddEntry("Software",3);
  DGScopeTriggerMode_CBL->GetComboBox()->Select(2);
  DGScopeTriggerMode_CBL->GetComboBox()->Resize(110,20);
  DGScopeTriggerMode_CBL->GetComboBox()->ChangeOptions(DGScopeTriggerMode_CBL->GetComboBox()->GetOptions() | kFixedSize);

  // ROOT text button for manually triggering of DGScope acquisition
  DGScopeTriggerControls_GF->AddFrame(DGScopeTrigger_TB = new TGTextButton(DGScopeTriggerControls_GF, "Manual trigger", DGScopeTrigger_TB_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeTrigger_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeTrigger_TB->Resize(120,25);
  DGScopeTrigger_TB->ChangeOptions(DGScopeTrigger_TB->GetOptions() | kFixedSize);

  
  ///////////////////////
  // Acquisition controls

  TGGroupFrame *DGScopeAcquisitionControls_GF = new TGGroupFrame(DGScopeSettingsFrame, "Acquisition", kVerticalFrame);
  DGScopeAcquisitionControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSettingsFrame->AddFrame(DGScopeAcquisitionControls_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ number entry specifying number of samples
  DGScopeAcquisitionControls_GF->AddFrame(DGScopeRecordLength_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Record length (#)", DGScopeRecordLength_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeRecordLength_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeRecordLength_NEL->GetEntry()->SetNumber(512);

  // ADAQ number entry specifying the percentage of the acquisition
  // window that is behind (or after) the triggern (all channels)
  DGScopeAcquisitionControls_GF->AddFrame(DGScopePostTriggerSize_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Post trigger (%)", DGScopePostTriggerSize_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
  DGScopePostTriggerSize_NEL->GetEntry()->SetLimitValues(0,100);
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumber(50);


  ///////////////////////
  // Spectrum settings //
  ///////////////////////

  ////////////
  // Histogram  

  TGGroupFrame *DGScopeSpectrumHistogram_GF = new TGGroupFrame(DGScopeSpectrumFrame, "Histogram", kVerticalFrame);
  DGScopeSpectrumHistogram_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumFrame->AddFrame(DGScopeSpectrumHistogram_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));

  // ADAQ combo box for selecting the channel for display spectrum
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumChannel_CBL = new ADAQComboBoxWithLabel(DGScopeSpectrumHistogram_GF, "", -1),
					new TGLayoutHints(kLHintsNormal,0,0,5,5));
  for(uint32_t ch=0; ch<8; ch++)
    DGScopeSpectrumChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  DGScopeSpectrumChannel_CBL->GetComboBox()->Select(0);
  
  // ADAQ number entry to specify number of bins used in the spectra histogram
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumBinNumber_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumHistogram_GF, "Number of bins  ", DGScopeSpectrumBinNumber_NEL_ID),
					new TGLayoutHints(kLHintsLeft,0,0,5,0));
  DGScopeSpectrumBinNumber_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumBinNumber_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumBinNumber_NEL->GetEntry()->SetNumber(100);

  // ADAQ number entry to specify the maximum bin in the spectra histogram
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumMinBin_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumHistogram_GF, "Minimum bin", DGScopeSpectrumMinBin_NEL_ID),
				    new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetNumber(0);

  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumMaxBin_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumHistogram_GF, "Maximum bin", DGScopeSpectrumMaxBin_NEL_ID),
				       new TGLayoutHints(kLHintsLeft,0,0,0,5));
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumber(30000);

  TGHorizontalFrame *DGScopeSpectrumAxis_HF = new TGHorizontalFrame(DGScopeSpectrumHistogram_GF);
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumAxis_HF, new TGLayoutHints(kLHintsNormal,0,0,0,0));

  // ROOT check button that allows multiple runs ("run" == acquisition
  // on and then off) to sum into the same histogram
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumAggregateRuns_CB = new TGCheckButton(DGScopeSpectrumHistogram_GF, "Aggregate runs", DGScopeSpectrumAggregateRuns_CB_ID),
					new TGLayoutHints(kLHintsLeft,0,0,5,5));


  /////////////////
  // Pulse analysis

  TGGroupFrame *DGScopeSpectrumAnalysis_GF = new TGGroupFrame(DGScopeSpectrumFrame,"Analysis");
  DGScopeSpectrumAnalysis_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumFrame->AddFrame(DGScopeSpectrumAnalysis_GF);

  TGButtonGroup *DGScopeSpectrumAnalysis_BG = new TGButtonGroup(DGScopeSpectrumAnalysis_GF,"",kHorizontalFrame);
  DGScopeSpectrumAnalysis_BG->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysis_BG, new TGLayoutHints(kLHintsNormal,-13,0,0,0));

  DGScopeSpectrumAnalysisHeight_RB = new TGRadioButton(DGScopeSpectrumAnalysis_BG, "PHS    ", -1);
  DGScopeSpectrumAnalysisArea_RB = new TGRadioButton(DGScopeSpectrumAnalysis_BG, "PAS", -1);
  DGScopeSpectrumAnalysisArea_RB->SetState(kButtonDown);
  
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLLD_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumAnalysis_GF, "LLD (ADC/keV)",-1),
				       new TGLayoutHints(kLHintsNormal, 0,0,-5,0));
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(0);

  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisULD_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumAnalysis_GF, "ULD (ADC/keV)",-1),
				       new TGLayoutHints(kLHintsNormal, 0,0,0,5));
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(100000);

  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLDTrigger_CB = new TGCheckButton(DGScopeSpectrumAnalysis_GF,"Use LD trigger on",-1),
				       new TGLayoutHints(kLHintsNormal,0,0,0,5));
  
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLDTriggerChannel_CBL = new ADAQComboBoxWithLabel(DGScopeSpectrumAnalysis_GF,"",-1),
				       new TGLayoutHints(kLHintsNormal,0,0,0,5));
  for(uint32_t ch=0; ch<8; ch++)
    DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->Select(0);
  
  //////////////
  // Calibration

  TGGroupFrame *DGScopeSpectrumCalibration_GF = new TGGroupFrame(DGScopeSpectrumFrame, "Calibration", kVerticalFrame);
  DGScopeSpectrumCalibration_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumFrame->AddFrame(DGScopeSpectrumCalibration_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));

  TGHorizontalFrame *DGScopeSpectrumCalibration_HF1 = new TGHorizontalFrame(DGScopeSpectrumCalibration_GF);
  DGScopeSpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibration_HF1);
  
  DGScopeSpectrumCalibration_HF1->AddFrame(DGScopeSpectrumCalibration_CB = new TGCheckButton(DGScopeSpectrumCalibration_HF1, "Engage!", DGScopeSpectrumCalibration_CB_ID),
					   new TGLayoutHints(kLHintsLeft,5,5,5,5));
  DGScopeSpectrumCalibration_CB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSpectrumCalibration_CB->SetState(kButtonUp);

  DGScopeSpectrumCalibration_HF1->AddFrame(DGScopeSpectrumCalibrationPoint_CBL = new ADAQComboBoxWithLabel(DGScopeSpectrumCalibration_HF1, "",-1),
					   new TGLayoutHints(kLHintsNormal, 20,5,5,5));
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Point 0",0);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
  
  // ADAQ number entry for setting the calibration energy. The
  // calibration energy is a known energy of detected particles (ie, a
  // gamma peak) to which the third slider of the DGScope horizontal
  // slider is directed on the DGScope spectrum. The calibration is
  // then computed by dividing the position of the slider in the pulse
  // height spectrum by the entered calibration energy to obtain
  // [number of bins / keV]
  DGScopeSpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationEnergy_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumCalibration_GF, "Energy (keV)  ", -1),
					  new TGLayoutHints(kLHintsLeft,0,0,5,0));
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);

  DGScopeSpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationPulseUnit_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumCalibration_GF, "Pulse Unit (ADC)", DGScopeSpectrumCalibrationPulseUnit_NEL_ID),
				       new TGLayoutHints(kLHintsLeft,0,0,0,5));
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);

  TGHorizontalFrame *DGScopeSpectrumCalibration_HF2 = new TGHorizontalFrame(DGScopeSpectrumCalibration_GF);
  DGScopeSpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibration_HF2,
					  new TGLayoutHints(kLHintsExpandX));
  
  // Set point text button
  DGScopeSpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationSetPoint_TB = new TGTextButton(DGScopeSpectrumCalibration_HF2, "Set Pt.", DGScopeSpectrumCalibrationSetPoint_TB_ID),
					   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeSpectrumCalibrationSetPoint_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSpectrumCalibrationSetPoint_TB->Resize(100,25);
  DGScopeSpectrumCalibrationSetPoint_TB->ChangeOptions(DGScopeSpectrumCalibrationSetPoint_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);
  
  // Calibrate text button
  DGScopeSpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationCalibrate_TB = new TGTextButton(DGScopeSpectrumCalibration_HF2, "Calibrate", DGScopeSpectrumCalibrationCalibrate_TB_ID),
					  new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeSpectrumCalibrationCalibrate_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSpectrumCalibrationCalibrate_TB->Resize(100,25);
  DGScopeSpectrumCalibrationCalibrate_TB->ChangeOptions(DGScopeSpectrumCalibrationCalibrate_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);

  TGHorizontalFrame *DGScopeSpectrumCalibration_HF3 = new TGHorizontalFrame(DGScopeSpectrumCalibration_GF);
  DGScopeSpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibration_HF3,
					  new TGLayoutHints(kLHintsExpandX));
  
  // Plot text button
  DGScopeSpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationPlot_TB = new TGTextButton(DGScopeSpectrumCalibration_HF3, "Plot", DGScopeSpectrumCalibrationPlot_TB_ID),
					   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeSpectrumCalibrationPlot_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSpectrumCalibrationPlot_TB->Resize(100,25);
  DGScopeSpectrumCalibrationPlot_TB->ChangeOptions(DGScopeSpectrumCalibrationPlot_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonDisabled);
  
  // Reset text button
  DGScopeSpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationReset_TB = new TGTextButton(DGScopeSpectrumCalibration_HF3, "Reset", DGScopeSpectrumCalibrationReset_TB_ID),
					   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeSpectrumCalibrationReset_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSpectrumCalibrationReset_TB->Resize(100,25);
  DGScopeSpectrumCalibrationReset_TB->ChangeOptions(DGScopeSpectrumCalibrationReset_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationReset_TB->SetState(kButtonDisabled);


  //////////////////////
  // Display settings //
  //////////////////////

  ////////////////////////////
  // Title names and positions 

  TGGroupFrame *DGScopeDisplayTitle_GF = new TGGroupFrame(DGScopeDisplaySettingsFrame, "Titles", kVerticalFrame);
  DGScopeDisplayTitle_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDisplaySettingsFrame->AddFrame(DGScopeDisplayTitle_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ text entries and number entries for specifying the DGScope title, axes title, and axes position

  // X axis options
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayXTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "X-axis title", -1),
				      new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeDisplayXTitle_TEL->GetEntry()->SetText("");

  TGHorizontalFrame *DGScopeDisplayXTitleOptions_HF = new TGHorizontalFrame(DGScopeDisplayTitle_GF);
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayXTitleOptions_HF, new TGLayoutHints(kLHintsLeft, 0,0,0,0));

  DGScopeDisplayXTitleOptions_HF->AddFrame(DGScopeDisplayXTitleSize_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayXTitleOptions_HF, "Size", -1),
					   new TGLayoutHints(kLHintsLeft,5,5,0,0));
  DGScopeDisplayXTitleSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayXTitleSize_NEL->GetEntry()->SetNumber(0.05);

  DGScopeDisplayXTitleOptions_HF->AddFrame(DGScopeDisplayXTitleOffset_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayXTitleOptions_HF, "Offset", -1),
					   new TGLayoutHints(kLHintsLeft,5,5,0,5));
  DGScopeDisplayXTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayXTitleOffset_NEL->GetEntry()->SetNumber(1.2);

  // Y-axis options
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayYTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "Y-axis title", -1),
				      new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeDisplayYTitle_TEL->GetEntry()->SetText("");

  TGHorizontalFrame *DGScopeDisplayYTitleOptions_HF = new TGHorizontalFrame(DGScopeDisplayTitle_GF);
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayYTitleOptions_HF, new TGLayoutHints(kLHintsLeft, 0,0,0,0));
  
  DGScopeDisplayYTitleOptions_HF->AddFrame(DGScopeDisplayYTitleSize_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayYTitleOptions_HF, "Size", -1),
				      new TGLayoutHints(kLHintsNormal,5,0,0,5));
  DGScopeDisplayYTitleSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayYTitleSize_NEL->GetEntry()->SetNumber(0.05);

  DGScopeDisplayYTitleOptions_HF->AddFrame(DGScopeDisplayYTitleOffset_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayYTitleOptions_HF, "Offset", -1),
				      new TGLayoutHints(kLHintsNormal,5,0,0,5));
  DGScopeDisplayYTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayYTitleOffset_NEL->GetEntry()->SetNumber(1.5);
  
  // Title options
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "Graph Title", -1),
				      new TGLayoutHints(kLHintsNormal,5,5,5,5));

  ///////////////////////
  // Graphical attributes
  
  TGGroupFrame *DGScopeDisplaySettings_GF = new TGGroupFrame(DGScopeDisplaySettingsFrame, "Display", kVerticalFrame);
  DGScopeDisplaySettings_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDisplaySettingsFrame->AddFrame(DGScopeDisplaySettings_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ROOT check button to enable/disable plotting of the legend
  DGScopeDisplaySettings_GF->AddFrame(DGScopeDisplayDrawLegend_CB = new TGCheckButton(DGScopeDisplaySettings_GF, "Legend", -1),
				      new TGLayoutHints(kLHintsNormal,0,0,5,0));

  // ROOT check buttons for specifying if X and Y axes on spectra should be logarithmic
  DGScopeDisplaySettings_GF->AddFrame(DGScopeSpectrumXAxisLog_CB = new TGCheckButton(DGScopeDisplaySettings_GF, "Log. X-axis  ", DGScopeSpectrumXAxisLog_CB_ID),
				      new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DGScopeDisplaySettings_GF->AddFrame(DGScopeSpectrumYAxisLog_CB = new TGCheckButton(DGScopeDisplaySettings_GF, "Log. Y-axis", DGScopeSpectrumYAxisLog_CB_ID),
				      new TGLayoutHints(kLHintsLeft,0,0,0,0));

  TGButtonGroup *DGScopeDisplayWaveformXAxis_BG = new TGButtonGroup(DGScopeDisplaySettings_GF,"Waveform X axis",kHorizontalFrame);
  DGScopeDisplaySettings_GF->AddFrame(DGScopeDisplayWaveformXAxis_BG, new TGLayoutHints(kLHintsNormal,5,0,5,0));
  
  DGScopeDisplayWaveformXAxisSample_RB = new TGRadioButton(DGScopeDisplayWaveformXAxis_BG, "Sample", -1);
  DGScopeDisplayWaveformXAxisSample_RB->SetState(kButtonDown);
  
  DGScopeDisplayWaveformXAxisNanoseconds_RB = new TGRadioButton(DGScopeDisplayWaveformXAxis_BG, "ns", -1);

  TGButtonGroup *DGScopeDisplayWaveformYAxis_BG = new TGButtonGroup(DGScopeDisplaySettings_GF,"Waveform Y axis",kHorizontalFrame);
  DGScopeDisplaySettings_GF->AddFrame(DGScopeDisplayWaveformYAxis_BG, new TGLayoutHints(kLHintsNormal,5,0,5,0));
  
  DGScopeDisplayWaveformYAxisADC_RB = new TGRadioButton(DGScopeDisplayWaveformYAxis_BG, "ADC", -1);
  DGScopeDisplayWaveformYAxisADC_RB->SetState(kButtonDown);
  
  DGScopeDisplayWaveformYAxisMillivolts_RB = new TGRadioButton(DGScopeDisplayWaveformYAxis_BG, "mV", -1);


  ///////////////////
  // Graphical output

  TGGroupFrame *DGScopeDisplayOutput_GF = new TGGroupFrame(DGScopeDisplaySettingsFrame, "Output", kVerticalFrame);
  DGScopeDisplayOutput_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDisplaySettingsFrame->AddFrame(DGScopeDisplayOutput_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ text entry for entering DGScope graphical output file name
  DGScopeDisplayOutput_GF->AddFrame(DGScopeDisplayOutputFileName_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayOutput_GF, "File name", -1),
				    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDisplayOutputFileName_TEL->GetEntry()->SetText("DGScopePlot");

  // ADAQ combo box for selecting the desired DGScope graphical output file format
  DGScopeDisplayOutput_GF->AddFrame(DGScopeDisplayOutputFileType_CBL = new ADAQComboBoxWithLabel(DGScopeDisplayOutput_GF, "File type", -1),
				    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDisplayOutputFileType_CBL->GetComboBox()->AddEntry(".eps",0);
  DGScopeDisplayOutputFileType_CBL->GetComboBox()->AddEntry(".ps",1);
  DGScopeDisplayOutputFileType_CBL->GetComboBox()->AddEntry(".png",2);
  DGScopeDisplayOutputFileType_CBL->GetComboBox()->AddEntry(".jpeg",3);
  DGScopeDisplayOutputFileType_CBL->GetComboBox()->Select(0);
  
  // ROOT text button that saves the DGScope canvas to file
  DGScopeDisplayOutput_GF->AddFrame(DGScopeSave_TB = new TGTextButton(DGScopeDisplayOutput_GF, "Save plot", DGScopeSave_TB_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeSave_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeSave_TB->Resize(100,25);
  DGScopeSave_TB->ChangeOptions(DGScopeSave_TB->GetOptions() | kFixedSize);

  
  //////////////////
  // Data storage //
  //////////////////

  TGVerticalFrame *DGScopeDataStorage_VF = new TGVerticalFrame(DGScopeDataStorageFrame);
  DGScopeDataStorageFrame->AddFrame(DGScopeDataStorage_VF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ text entry for setting the ROOT file name
  DGScopeDataStorage_VF->AddFrame(DGScopeDataFileName_TEL = new ADAQTextEntryWithLabel(DGScopeDataStorage_VF, "Filename (.root)",-1),
				    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDataFileName_TEL->GetEntry()->SetText("ADAQMeasurement.root");
  DGScopeDataFileName_TEL->GetEntry()->SetState(false);

  // ADAQ text entry that will add a comment on the saved data to the ROOT file, making it possible to
  // record various parameters, settings, etc on the measurement for later retrieval
  DGScopeDataStorage_VF->AddFrame(DGScopeDataComment_TEL = new ADAQTextEntryWithLabel(DGScopeDataStorage_VF, "Comment", -1),
				    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDataComment_TEL->GetEntry()->SetState(false);

  // ROOT text button to create a root file using the name in the text entry field above
  DGScopeDataStorage_VF->AddFrame(DGScopeDataStorageCreateFile_TB = new TGTextButton(DGScopeDataStorage_VF,"Create ROOT file", DGScopeDataStorageCreateFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeDataStorageCreateFile_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeDataStorageCreateFile_TB->Resize(150,25);
  DGScopeDataStorageCreateFile_TB->ChangeOptions(DGScopeDataStorageCreateFile_TB->GetOptions() | kFixedSize);
  DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);

  // ROOT text button to write all data to the ROOT file and close it. This button MUST be clicked to 
  // successfully write&close the ROOT file otherwise the ROOT file will have errors.
  DGScopeDataStorage_VF->AddFrame(DGScopeDataStorageCloseFile_TB = new TGTextButton(DGScopeDataStorage_VF,"Close ROOT file", DGScopeDataStorageCloseFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeDataStorageCloseFile_TB->Connect("Clicked()","ADAQAcquisitionGUI",this,"HandleScopeButtons()");
  DGScopeDataStorageCloseFile_TB->Resize(150,25);
  DGScopeDataStorageCloseFile_TB->ChangeOptions(DGScopeDataStorageCloseFile_TB->GetOptions() | kFixedSize);
  DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
  
  // ROOT check button to enable/disable saving data to ROOT file. Note that the data is saved to
  // the ROOT file only while the button is checked. The 
  DGScopeDataStorage_VF->AddFrame(DGScopeDataStorageEnable_CB = new TGCheckButton(DGScopeDataStorage_VF,"Data stored while checked", -1),
				  new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);


  ///////////////////
  // Miscellaneous //
  ///////////////////

  TGVerticalFrame *DGScopeMisc1_VF = new TGVerticalFrame(DGScopeMiscFrame);
  DGScopeMiscFrame->AddFrame(DGScopeMisc1_VF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  DGScopeMisc1_VF->AddFrame(DGScopeSpectrumRefreshRate_NEL = new ADAQNumberEntryWithLabel(DGScopeMisc1_VF,"Spectrum Refresh Rate", -1),
			    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumber(100);
  

  DGScopeDisplayAndControls_VF->AddFrame(DGScopeDisplay_GF, new TGLayoutHints(kLHintsCenterX,5,5,5,0));
  DGScopeDisplayAndControls_VF->AddFrame(DGScopeStartStop_TB, new TGLayoutHints(kLHintsCenterX,0,0,0,5));
  DGScopeDisplayAndControls_VF->AddFrame(DGScopeControlTabs_HF, new TGLayoutHints(kLHintsCenterX,5,5,0,5));

  ScopeFrame->AddFrame(DGScopeDisplayAndControls_VF, new TGLayoutHints(kLHintsNormal,5,5,5,5));
}


// Perform actions triggers by the text buttons on the Connection
// Frame, which is principally connecting the ADAQAcquisitionGUI to the VME
// boards as well as reading/writing registers on individual boards
void ADAQAcquisitionGUI::HandleConnectionButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();

  // 32-bit integers to hold register addresses and data; 16-bit
  // integer to hold data obtained from the V6534 board
  uint32_t addr32 = 0;
  uint32_t data32 = 0;
  uint16_t data16 = 0;

  switch(ActiveButtonID){

    // Connect ADAQAcquisitionGUI with VME boards
  case V1718Connect_TB_ID:
    
    // If no connection is presently established...
    if(!VMEConnectionEstablished){

      int V6534LinkOpen = -1;
      int V1720LinkOpen = -1;
      
      // ... check to see if the HVManager exists, indicating that the
      // V6534 board is available for use. If found, get the physical
      // address of V6534 board and use the HVManager to initialize it
      if(V6534Enable){
	V6534BoardAddress = V6534BoardAddress_NEF->GetHexNumber();
	V6534LinkOpen = HVManager->OpenLink(V6534BoardAddress);
	HVManager->SetToSafeState();
      }

      // ... check to see if the DgManager exists, indicating that the
      // V1720 board is available for use. If found, get the physical
      // address of V1720 board and use the HVManager to initialize it
      if(V1720Enable){
	V1720BoardAddress = V1720BoardAddress_NEF->GetHexNumber();
	V1720LinkOpen = DGManager->OpenLink(V1720BoardAddress);
	DGManager->Initialize();
      }
      
      // If either (or both) of the HV and DG managers exist then
      // attempt to make the connection, changing the button text and
      // color. 

      // ZSH: This needs to be done much better, checking for an
      // established connection before setting button attributes. I
      // plan on getting to this but...well...other shit to do...
      if(V6534LinkOpen==0 or V1720LinkOpen==0){
	V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
	V1718Connect_TB->SetText("Connected: click to disconnect");
	VMEConnectionEstablished = true;
      }
      else
	V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
    }

    // If a connection is already established then terminate the connection
    else if(VMEConnectionEstablished){

      // Call the functions that accomplishes safe shutdown of the
      // V6534 and V1720 boards (powering down voltages, turning all
      // channels off, etc)
      HandleDisconnectAndTerminate(false);

      // Change the color and text of the button.
      V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
      V1718Connect_TB->SetText("Disconnected: click to connect");
      VMEConnectionEstablished = false;
    }
    break;

    // Set the V6534Enable boolean that controls whether or not the
    // V6534 high voltage board should be presently used
  case V6534BoardEnable_TB_ID:
    if(V6534BoardEnable_TB->GetString() == "Board enabled"){
      V6534BoardEnable_TB->SetText("Board disabled");
      V6534BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
      V6534Enable = false;
    }
    else if(V6534BoardEnable_TB->GetString() == "Board disabled"){
      V6534BoardEnable_TB->SetText("Board enabled");
      V6534BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
      V6534Enable = true;
    }
    break;

    // Set the V1720Enable boolean that controls whether or not the
    // V1720 high voltage board should be presently used
  case V1720BoardEnable_TB_ID:
    if(V1720BoardEnable_TB->GetString() == "Board enabled"){
      V1720BoardEnable_TB->SetText("Board disabled");
      V1720BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
      V1720Enable = false;
    }
    else if(V1720BoardEnable_TB->GetString() == "Board disabled"){
      V1720BoardEnable_TB->SetText("Board enabled");
      V1720BoardEnable_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
      V1720Enable = true;
    }
    break;

    // Read the specified register on the V1720 board and update the
    // ROOT number entry widget to display the register value
  case V1720Read_ID:
    addr32 = V1720ReadAddress_NEF->GetHexNumber();
    if(V1720Enable) DGManager->GetRegisterValue(addr32, &data32);
    V1720ReadValue_NEF->SetHexNumber(data32);
    break;
    
    // Write the specified value to the specified register on the V1720 board
  case V1720Write_ID:
    addr32 = V1720WriteAddress_NEF->GetHexNumber();
    data32 = V1720WriteValue_NEF->GetHexNumber();
    if(V1720Enable) DGManager->SetRegisterValue(addr32, data32);
    break;

    // Read the specified register on the V6534 board and update the
    // ROOT number entry widget to display the register value
  case V6534Read_ID:
    addr32 = V6534ReadAddress_NEF->GetHexNumber();
    if(V6534Enable) HVManager->GetRegisterValue(addr32,&data16);
    V6534ReadValue_NEF->SetHexNumber(data16);
    break;

    // Write the specified value to the specified register on the V6534 board
  case V6534Write_ID:
    addr32 = V6534WriteAddress_NEF->GetHexNumber();
    data16 = V6534WriteValue_NEF->GetHexNumber();
    if(V6534Enable) HVManager->SetRegisterValue(addr32,data16);
    break;

  default:
    cout << "\nError in HandleConnectionButtons(): Unknown button ID!\n" << endl;
    break;
  }
}


// Perform actions that are activated by the text buttons on the HV frame
void ADAQAcquisitionGUI::HandleVoltageButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();

  // Return if the user has specified that the V6534 high voltage
  // board should not be used during this session
  if(!V6534Enable)
    return;
  
  switch(ActiveButtonID){
    
    ///////////////////////
    // HV Power Settings 
    
  case HVChannel0Power_TB_ID:
  case HVChannel1Power_TB_ID:
  case HVChannel2Power_TB_ID:
  case HVChannel3Power_TB_ID:
  case HVChannel4Power_TB_ID:
  case HVChannel5Power_TB_ID:{
    
    // Determine the HV channel number corresponding to the clicked button
    int HVChannel = HVChannelPower_TB_ID_Map[ActiveButtonID];

    // If the power is being turned from "OFF" to "ON"...
    if(ActiveTextButton->GetString()=="OFF"){
      // Update the button color from red to green and set text to "ON"
      ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(8));
      ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
      ActiveTextButton->SetText("ON");
      
      // Get the voltage and maximum current settings from the ROOT
      // number entry widgets for the desired HV channel
      int HVVoltageValue = HVChannelV_NEL[HVChannel]->GetEntry()->GetIntNumber();
      int HVCurrentValue = HVChannelI_NEL[HVChannel]->GetEntry()->GetIntNumber();
      
      // Set the voltage and maxmimum current drawn and turn the HV channel on
      HVManager->SetVoltage(HVChannel,HVVoltageValue); 
      HVManager->SetCurrent(HVChannel,HVCurrentValue);
      HVManager->SetPowerOn(HVChannel);

      // Disable the state of the HV widgets corresponding to the
      // channel that has just been turned on. 
      //
      // ZSH: We may want to update this feature to allow dynamic
      // setting of the channel voltage and current
      SetHVWidgetState(HVChannel, true);
    }

    // If the power is being turned from "ON" to "OFF"...
    else if(ActiveTextButton->GetString()=="ON"){
      // Update the button color from green to red and set text to "OFF"
      ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(2));
      ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
      ActiveTextButton->SetText("OFF");

      // Turn the HV channel off
      HVManager->SetPowerOff(HVChannel);

      // Reenable the widget status such that voltage and maximum
      // current can be modified
      SetHVWidgetState(HVChannel, false);
    }
    break;
  }

    //////////////////
    // HV Monitoring 
    
    // Enable real-time monitoring of all channel's voltage and drawn current
  case HVEnableMonitoring_CB_ID:{
    
    // If monitoring is being turned on....
    if(HVMonitorEnable_CB->IsDown()){
      
      // Set bool to enable the HV monitoring loop (see
      // ADAQAcquisitionGUI::RunHVMonitoring)
      HVMonitorEnable = true;

      // Enable the HV channel monitoring ROOT number entry field
      // widgets to show that monitoring is turned on
      for(int ch=0; ch<HVManager->GetNumChannels(); ch++){
	HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(true);
	HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(true);
      }

      // Run the HV monitoring
      RunHVMonitoring();
    }

    // If monitoring is being turned off...
    else{

      // Disable the HV channel monitoring ROOT number entry field
      // widgets to show that monitoring is turned off
      for(int ch=0; ch<HVManager->GetNumChannels(); ch++){
	HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(false);
	HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(false);
      }

      // Set bool to disable HV monitoring loop
      HVMonitorEnable = false;
      break;
    }
  }
    
  default:
    break;
    
  }
}


// Perform actions triggers by the text buttons on the Scope Frame
void ADAQAcquisitionGUI::HandleScopeButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();
  
  // Return if the user has specified that the V1720 digitizer board
  // should be used during this session
  if(!V1720Enable)
    return;
  
  switch(ActiveButtonID){
    
    ///////////////////////////////////
    // Start/Stop DGScope acquisition
  case DGScopeStartStop_TB_ID:{
    
    // If DGScope acquisition is started
    if(ActiveTextButton->GetString()=="Stopped"){
      // Update button color from red to green and update text to "Stop"
      ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(8));
      ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
      ActiveTextButton->SetText("Acquiring");

      // Set bool to enable the DGScope to update in real time (see
      // ADAQAcquisitionGUI::RunDGScope) during data acquisition
      DGScopeEnable = true;
      
      // Set the active ROOT canvas to the DGScope embedded canvas
      DGScope_EC->GetCanvas()->cd(0);
      
      // Disable certain widgets while DGScope is acquiring
      // data. These widgets tend to have values that are used to set
      // V1720 parameters at the beginning of an acquisition cycle
      // and, at present, cannot be changed during an acquisition
      // cycle. Thus, disabling these widgets indicates to the user
      // that setting them can only be done while DGScope is not active
      SetDGWidgetState(true);

      // Run the DGScope
      RunDGScope();
    }

    // If DGScope acquisition is being stopped...
    else if(ActiveTextButton->GetString()=="Acquiring"){
      // Update button color from green to red and update text to "Start"
      ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(2));
      ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
      ActiveTextButton->SetText("Stopped");

      // Set bool to disable the DGScope loop, reenable all widgets,
      // and stop the digitizer from acquiring data
      DGScopeEnable = false;
      SetDGWidgetState(false);
      DGManager->SWStopAcquisition();

      // Determine if a ROOT file was open and receiving data; if so,
      // ensure that the data is written and the ROOT file is closed
      /*
      if(OutputDataFile)
	if(OutputDataFile->IsOpen())
	  DGScopeDataStorageCloseFile_TB->Clicked();
      */
    }
  }
    
    // Send a software signal to the V1720 to for a manually forced
    // data trigger across all channels
  case DGScopeTrigger_TB_ID:{
    DGManager->SendSWTrigger();
    break;
  }

    //////////////////////////
    // Save DGScope graphics
  case DGScopeSave_TB_ID:{
    
    // Obtain the base name (no "." or file extension) from the ROOT text entry widget
    string fBaseName = DGScopeDisplayOutputFileName_TEL->GetEntry()->GetText();

    // Obtain the file type and extension from the ROOT combo box widget
    string fExtension = "";
    int PlotType = DGScopeDisplayOutputFileType_CBL->GetComboBox()->GetSelected();
    switch(PlotType){
    case 0:
      fExtension.assign(".eps");
      break;
    case 1:
      fExtension.assign(".ps");
      break;
    case 2:
      fExtension.assign(".png");
      break;
    case 3:
      fExtension.assign(".jpg");
      break;
    }

    // Perform a check to ensure that the specified total file name
    // does not exist to prevent overwriting important graphics; if
    // the file does exist, increment an integer counter that is added
    // onto the file num to create a new but recognizable file name
    int num = 0;
    stringstream ss;
    string fNumber;
    string fName;

    bool fExists = true;
    while(fExists){
	  ss << num;
	  fNumber.assign(ss.str());
	  fName = fBaseName+fNumber+fExtension;
	  ifstream fCheck(fName.c_str());
	  fExists = (bool)fCheck;
	  ss.str("");
	  num++;
    }

    // Print the state of the DGScope embedded canvas to file
    DGScope_EC->GetCanvas()->Print(fName.c_str(), fExtension.c_str());
    break;
  }
    
    // Enable the calibration widgets
  case DGScopeSpectrumCalibration_CB_ID:
    if(DGScopeSpectrumCalibration_CB->IsDown()){
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(true);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(true);
      DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationReset_TB->SetState(kButtonUp);

    }

    // Disable the calibration widgets
    else{
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
      DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationReset_TB->SetState(kButtonDisabled);
    }
    break;

    /////////////////////////////////////////////////
    // Add a new DGScope spectrum calibration point
  case DGScopeSpectrumCalibrationSetPoint_TB_ID:{

    // Get the calibration point to be set
    uint SetPoint = DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->GetSelected();

    // Get the energy of the present calibration point
    double Energy = DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();

    // Get the pulse unit value of the present calibration point
    int PulseUnit = DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

    if(SetPoint == CalibrationData[CurrentChannel].PointID.size()){

      CalibrationData[CurrentChannel].PointID.push_back(SetPoint);
      CalibrationData[CurrentChannel].Energy.push_back(Energy);
      CalibrationData[CurrentChannel].PulseUnit.push_back(PulseUnit);
      
      // Add a new point to the number of calibration points in case
      // the user wants to add subsequent points to the calibration
      stringstream ss;
      ss << (SetPoint+1);
      string NewPointLabel = "Point " + ss.str();
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(NewPointLabel.c_str(),SetPoint+1);
      
      // Set the combo box to display the new calibration point...
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(SetPoint+1);
      
      // ...and set the calibration energy and pulse unit ROOT number
      // entry widgets to their default "0.0" and "1.0" respectively,
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
    }

    // ...or if the user is re-setting previously set points then
    // simply overwrite the preexisting values in the vectors
    else{
      CalibrationData[CurrentChannel].Energy[SetPoint] = Energy;
      CalibrationData[CurrentChannel].PulseUnit[SetPoint] = PulseUnit;
    }
    break;
  }

    ///////////////////////////////////////////////
    // Create a new DGScope spectrum calibration
  case DGScopeSpectrumCalibrationCalibrate_TB_ID:{

    // If there are 2 or more points in the current channel's
    // calibration data set then create a new TGraph object. The
    // TGraph object will have pulse units [ADC] on the X-axis and the
    // corresponding energies [in whatever units the user has entered
    // the energy] on the Y-axis. A TGraph is used because it provides
    // very easy but extremely powerful methods for interpolation,
    // which allows the pulse height/area to be converted in to energy
    // efficiently in the acquisition loop.

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(CalibrationData[CurrentChannel].PointID.size() >= 2){

      // Determine the total number of calibration points in the
      // current channel's calibration data set
      const int NumCalibrationPoints = CalibrationData[CurrentChannel].PointID.size();

      // Create a new "CalibrationManager" TGraph object.
      CalibrationManager[CurrentChannel] = new TGraph(NumCalibrationPoints,
						      &CalibrationData[CurrentChannel].PulseUnit[0],
						      &CalibrationData[CurrentChannel].Energy[0]);
      
      // Set the current channel's calibration boolean to true,
      // indicating that the current channel will convert pulse units
      // to energy within the acquisition loop before histogramming
      // the result into the channel's spectrum
      UseCalibrationManager[CurrentChannel] = true;
    }
    break;
  }
    
  case DGScopeSpectrumCalibrationPlot_TB_ID:{
    
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(UseCalibrationManager[CurrentChannel]){
      TCanvas *Calibration_C = new TCanvas("Calibration_C","CalibrationManager TGraph",0,0,600,400);
      
      stringstream ss;
      ss << "CalibrationManager TGraph for Channel[" << CurrentChannel << "]";
      string Title = ss.str();

      CalibrationManager[CurrentChannel]->SetTitle(Title.c_str());
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitle("Pulse unit [ADC]");
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitle("Energy");
      CalibrationManager[CurrentChannel]->SetMarkerSize(2);
      CalibrationManager[CurrentChannel]->SetMarkerStyle(20);
      CalibrationManager[CurrentChannel]->Draw("ALP");

      Calibration_C->Update();

      DGScope_EC->GetCanvas()->cd(0);
    }
    break;
  }



  case DGScopeSpectrumCalibrationReset_TB_ID:{

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

    // Clear the channel calibration vectors for the current channel
    CalibrationData[CurrentChannel].PointID.clear();
    CalibrationData[CurrentChannel].Energy.clear();
    CalibrationData[CurrentChannel].PulseUnit.clear();

    // Delete the current channel's depracated calibration manager
    // TGraph object to prevent memory leaks
    if(UseCalibrationManager[CurrentChannel])
      delete CalibrationManager[CurrentChannel];

    // Set the current channel's calibration boolean to false,
    // indicating that the calibration manager will NOT be used within
    // the acquisition loop
    UseCalibrationManager[CurrentChannel] = false;
    
    // Reset the calibration widgets
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->RemoveAll();
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Point 0", 0);
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
    DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
    DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
    

    break;
  }
    
    

    ///////////////////////////
    // Create ROOT data file
  case DGScopeDataStorageCreateFile_TB_ID:{

    /////////////////////////////////////////////
    // Instantiate objects for persistent storage

    // Test to ensure that the specified TFile name does not exist to
    // prevent overwritting potentially precious data. You'll thank me
    // for this someday.
    ifstream TestFile(DGScopeDataFileName_TEL->GetEntry()->GetText());
    if(TestFile.is_open()){
      cout << "\nADAQ: Attempted to create a new ROOT file but one already exists with\n"
		<<   "      that name! Please specify a new ROOT file!\n"
		<< endl;
      TestFile.close();
      break;
    }
    TestFile.close();

    // TFile to create a ROOT binary file for output
    OutputDataFile = new TFile(DGScopeDataFileName_TEL->GetEntry()->GetText(),"recreate");
    
    // TTree to store the waveforms as arrays. The array indices are
    // sample numbers and array values are the voltages
    WaveformTree = new TTree("WaveformTree","Prototype tree to store all waveforms of an event");

    // TObjString to hold a comment on the measurement data
    MeasComment = new TObjString(DGScopeDataComment_TEL->GetEntry()->GetText());

    // ADAQ class to hold measurement paremeters
    MeasParams = new ADAQRootMeasParams();

    
    /////////////////////////////////////////////
    // Retrieve all values (except the waveforms)

    // Retrieve the present voltage and drawn current for each
    // high voltage channel and store in the MeasParam object
    uint16_t voltage = 0;
    uint16_t current = 0;

    for(int ch=0; ch<HVManager->GetNumChannels(); ch++){
      if(V6534Enable){
	MeasParams->DetectorVoltage.push_back( HVManager->GetVoltage(ch,&voltage) );
	MeasParams->DetectorCurrent.push_back( HVManager->GetCurrent(ch,&current) );
      }
      else{
	MeasParams->DetectorVoltage.push_back(0);
	MeasParams->DetectorCurrent.push_back(0);
      }
    }
    
    // Retrieve the present settings for each of the digitizer
    // channels and store in the MeasParam object
    for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
      MeasParams->DCOffset.push_back( (int)DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());
      MeasParams->TriggerThreshold.push_back( (int)DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() );
      MeasParams->BaselineCalcMin.push_back( (int)DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber() );
      MeasParams->BaselineCalcMax.push_back( (int)DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber() );
    }
    
    // Retrieve the record length for the acquisition window [samples]
    MeasParams->RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();


    ///////////////////////////
    // Set the infamous boolean

    // Set a bool indicating that the next digitized event will
    // trigger the creation of a TTree branch with the correctly sized
    // array. This action is performed once in
    // ADAQAcquisitionGUI::RunDGScope(). See that function for more comments
    BranchWaveformTree = true;


    //////////////////////////////////
    // Set widget states appropriately

    // Disable the filename, comment, and create file button (since we
    // don't want to create new ROOT files until the active is closed)
    // and activate the close file and enable buttons (since these
    // options are now available with an open ROOT file for data writing)
    DGScopeDataFileName_TEL->GetEntry()->SetState(false);
    DGScopeDataComment_TEL->GetEntry()->SetState(false);
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageCloseFile_TB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);

    break;
  }


    ///////////////////////////////
    // Write and close ROOT file
  case DGScopeDataStorageCloseFile_TB_ID:{

    if(WaveformTree)
      WaveformTree->Write();

    // Write the ROOT objects to file
    MeasParams->Write("MeasParams");
    MeasComment->Write("MeasComment");
        
    // Close the ROOT TFile
    OutputDataFile->Close();
    
    // Free the memory allocated to ROOT objects
    delete MeasComment;
    //EventTree->Delete(); // Causes crash for some reason
    delete MeasParams;
    delete OutputDataFile;

    // Set widget states appropriately.
    DGScopeDataFileName_TEL->GetEntry()->SetState(true);
    DGScopeDataComment_TEL->GetEntry()->SetState(true);
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
    DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);

    break;
  }

  default:
    break;
  }
}


// Perform actions triggers by DGScope number entries
void ADAQAcquisitionGUI::HandleScopeNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveEntryID = ActiveEntry->WidgetId();

  switch(ActiveEntryID){

    // Set the channel trigger thresholds
  case DGScopeCh0TriggerThreshold_NEL_ID:
  case DGScopeCh1TriggerThreshold_NEL_ID:
  case DGScopeCh2TriggerThreshold_NEL_ID:
  case DGScopeCh3TriggerThreshold_NEL_ID:
  case DGScopeCh4TriggerThreshold_NEL_ID:
  case DGScopeCh5TriggerThreshold_NEL_ID:
  case DGScopeCh6TriggerThreshold_NEL_ID:
  case DGScopeCh7TriggerThreshold_NEL_ID:{

    // Only enable setting if digitizer is currently acquiring
    if(DGScopeEnable){

      // Get the channel number to be set
      uint32_t ch = DGScopeChTriggerThreshold_NEL_ID_Map[ActiveEntryID];
      
      // Get the trigger threshold value [ADC] to be set
      uint32_t thr = DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
      
      // Set the channel trigger threshold 
      DGManager->SetChannelTriggerThreshold(ch, thr);
    }
  }
    
    // If the user enters the pulse unit in the provided number entry
    // widget (which is updated as the user slides the pointer of the
    // horizontal triple widget just under DGScope canvas) then update
    // the position of the horizontal triple widget such that the
    // drawn calibration line on th ecanvas matches the entered number
  case DGScopeSpectrumCalibrationPulseUnit_NEL_ID:
    int CalibrationPulseUnit = DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    DGScopeHorizontalScale_THS->SetPointerPosition(CalibrationPulseUnit);
    break;

 
  }
}
  

// Performm actions that ensure a safe shutdown and disconnect of the
// ADAQAcquisitionGUI software from the VME boards
void ADAQAcquisitionGUI::HandleDisconnectAndTerminate(bool TerminateApplication)
{
  // If the HVManager has been instantiated (ie, the V6534 is being
  // used) then set the V6534 board to a safe state (all
  // voltages/currents to 0, power to off) and then close the link to
  // the V6534 board
  if(V6534Enable){
    HVManager->SetToSafeState();
    HVManager->CloseLink();
  }
  
  // If the DGManager has been instatiated (ie, the V1720 is being
  // used) close the link to the V1720 board
  if(V1720Enable)
    DGManager->CloseLink();
  
  // Close the standalone ROOT application
  if(TerminateApplication)
    gApplication->Terminate();
}


// Set the state of the HV voltage and maximum current entry
// widgets. When a specific channel is turned "on", it's HV and I
// setting widgets are turned "off"
void ADAQAcquisitionGUI::SetHVWidgetState(int HVChannel, bool HVActive)
{
  bool WidgetState = true;
  if(HVActive)
    WidgetState = false;

  HVChannelV_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
  HVChannelI_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
}


// Set the state of DGScope widgets that are used to set parameters to
// program the V1720 digitizer at the beginning of a data acquisition
// session. These widgets are disabled when DGScope is acquiring.
void ADAQAcquisitionGUI::SetDGWidgetState(bool AcquiringData)
{
  EButtonState ButtonState = kButtonUp;
  bool WidgetState = true;
  if(AcquiringData){
    ButtonState = kButtonDisabled;
    WidgetState = false;
  }

  for(uint32_t ch=0; ch<8; ch++){
    DGScopeChannelEnable_CB[ch]->SetState(ButtonState,true);
    DGScopeDCOffset_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->SetState(WidgetState);
  }

  DGScopeTriggerCoincidenceEnable_CB->SetState(ButtonState,true);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(WidgetState);
  DGScopeTriggerMode_CBL->GetComboBox()->SetEnabled(WidgetState);

  DGScopeRecordLength_NEL->GetEntry()->SetState(WidgetState);
  DGScopePostTriggerSize_NEL->GetEntry()->SetState(WidgetState);

  DGScopeWaveform_RB->SetEnabled(WidgetState);
  DGScopeSpectrum_RB->SetEnabled(WidgetState);
  DGScopeBlank_RB->SetEnabled(WidgetState);

  DGScopeSpectrumBinNumber_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetState(WidgetState);

  DGScopeSpectrumAnalysisHeight_RB->SetEnabled(WidgetState);
  DGScopeSpectrumAnalysisArea_RB->SetEnabled(WidgetState);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetState(WidgetState);

  DGScopeSpectrumAggregateRuns_CB->SetState(ButtonState,true);

  DGScopeDataFileName_TEL->GetEntry()->SetState(AcquiringData);
  DGScopeDataComment_TEL->GetEntry()->SetState(AcquiringData);

  if(AcquiringData)
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
  else
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
}


// Run the real-time updating of the ROOT number entry widgets that
// display active voltage and drawn current from all channels
void ADAQAcquisitionGUI::RunHVMonitoring()
{
  // The high voltage and current will be displayed and updated in the
  // dedicated number entry fields when HVMonitorEnable is true
  while(HVMonitorEnable){
    // Perform action in a separate thread to enable use of other GUI
    // features while HV monitoring is taking place
    gSystem->ProcessEvents();
    
    // Update the voltage and current displays every second
    double delay = clock()+(1.0*CLOCKS_PER_SEC);
    while(clock()<delay){gSystem->ProcessEvents();}

    uint16_t Voltage, Current;
    for(int ch=0; ch<HVManager->GetNumChannels(); ch++){
      // Get the present active voltage and current values
      HVManager->GetVoltage(ch, &Voltage);
      HVManager->GetCurrent(ch, &Current);
      
      // Update the appropriate number entry fields
      HVChannelVMonitor_NEFL[ch]->GetEntry()->SetNumber(Voltage);
      HVChannelIMonitor_NEFL[ch]->GetEntry()->SetNumber(Current);
    }
  }
}


// Run the real-time updating of the DGScope embedded canvas
void ADAQAcquisitionGUI::RunDGScope()
{
  ///////////////////////////////////////////////////////////////
  // Declaring and setting digitizer variables for acquisition //
  ///////////////////////////////////////////////////////////////
  

  ///////////////////////////////////////////////
  // Variables for digitizer settings and readout
  
  // CAEN_DGTZ type variables for readout of the digitized waveforms
  // from the V1720 FPGA buffer onto the PC buffer
  char *EventPointer = NULL;
  CAEN_DGTZ_EventInfo_t EventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *EventWaveform = NULL;
  
  // Variables for PC buffer
  char *Buffer = NULL;
  uint32_t BufferSize;
  uint32_t Size, NumEvents;

  // Get the record length, ie, number of 4ns samples in acquisition window
  uint32_t RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();

  // Get the percentage of acquisition window that occurs after the trigger
  uint32_t PostTriggerSize = DGScopePostTriggerSize_NEL->GetEntry()->GetIntNumber();

  // Variables to record the digitized waveform. Raw units are used:
  // time is in sample number and voltage is in ADC
  double Time[RecordLength], Voltage[RecordLength]; // [sample], [ADC]

  // Variables for graphing the digitized waveforms as time versus
  // voltage. Units are determined...
  double Time_graph[RecordLength], Voltage_graph[RecordLength]; 

  // Variables for channel trigger thresholds, calculation of the
  // channel baselines
  uint32_t ChannelTriggerThreshold[8]; // [ADC]
  uint32_t BaselineCalcMin[8], BaselineCalcMax[8], BaselineCalcLength[8]; // [sample]
  double BaselineCalcResult[8]; // [ADC]

  // Variable to hold the channel enable mask, ie, sets which
  // digitizer channels are actively taking data
  uint32_t ChannelEnableMask = 0;

  // Variable for total number of enabled digitizer channels
  uint32_t NumDGChannelsEnabled = 0;

  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){

    // Get each channel's trigger threshold]
    ChannelTriggerThreshold[ch] = DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber(); // [ADC]

    // Get each channel's baseline calculation region (min, max, length)
    BaselineCalcMin[ch] = DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber(); // [sample]
    BaselineCalcMax[ch] = DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber(); // [sample]
    BaselineCalcLength[ch] = BaselineCalcMax[ch]-BaselineCalcMin[ch]; // [sample]
    BaselineCalcResult[ch] = 0; // [ADC] Result is calculated during acquisition

    // Calculate the channel enable mask, which is a 32-bit integer
    // describing which of the 8 digitizer channels are enabled. A
    // 32-bit integer has 8 bytes or 8 "hex" digits; a hex digit set
    // to "1" in the n-th position in the hex representation indicates
    // that the n-th channel is enabled. For example, if the
    // ChannelEnableMask is equal to 0x00110100 then channels 2, 4 and
    // 5 are enabled for digitization
    if(DGScopeChannelEnable_CB[ch]->IsDisabledAndSelected()){
      uint32_t Ch = 0x00000001<<ch;
      ChannelEnableMask |= Ch;
      NumDGChannelsEnabled++;
    }
  }
  
  // Ensure that at least one channel is enabled in the channel
  // enabled bit mask; if not, return without starting the acquisition
  // loop, since...well...there ain't shit to acquisition.
  if((0xff & ChannelEnableMask)==0)
    return;
  
  // Determine the lowest (ie, closest of 0) channel that is enabled
  // in the ChannelEnableMask. 'Fraid you'll have to figure out the
  // bitwise operations on your own if you don't know them...but take
  // it from me, this is a pretty pro'n'shit way to do it
  int LowestEnabledChannel = 0;
  uint32_t LowestChannelMask = 0x1;
  while(!(ChannelEnableMask & LowestChannelMask)){
    LowestChannelMask <<= 1;
    LowestEnabledChannel++;
  }
  
  // Get the desired triggering mode (external, automatic, or software)
  int TriggerMode = DGScopeTriggerMode_CBL->GetComboBox()->GetSelected(); 
  
  // Get the trigger coincidence level (number of channels in coincidence - 1);
  uint32_t TriggerCoincidenceLevel = DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->GetSelected();
  
  // Bit shift the coincidence level bits into position 24:26 of a
  // 32-bit integer such that they may be added to the digitizer's
  // TriggerSourceEnableMask with the "or" bit operator
  uint32_t TriggerCoincidenceLevel_BitShifted = TriggerCoincidenceLevel << 24;
 
 
  /////////////////////////////////////////////////
  // Variables for waveform/spectrum graph settings

  // Get the number of, min, and max bins for the spectrum
  int bins = DGScopeSpectrumBinNumber_NEL->GetEntry()->GetIntNumber();
  int minBin = DGScopeSpectrumMinBin_NEL->GetEntry()->GetIntNumber();
  int maxBin = DGScopeSpectrumMaxBin_NEL->GetEntry()->GetIntNumber();

  // Allocate variables for the X and Y minimum and maximum positions
  float xMin, xMax, yMin, yMax;
  
  // Get the bools to determine what (if anything) is plotted
  bool PlotWaveform = DGScopeWaveform_RB->IsDown();
  bool PlotSpectrum = DGScopeSpectrum_RB->IsDown();
  bool PlotBlank = DGScopeBlank_RB->IsDown();

  // Get the bools to determine plotting options
  bool DrawLegend = DGScopeDisplayDrawLegend_CB->IsDown();

  bool PlotXAxisInSamples = DGScopeDisplayWaveformXAxisSample_RB->IsDown();
  bool PlotYAxisInADC = DGScopeDisplayWaveformYAxisADC_RB->IsDown();
  double ConvertTimeToGraphUnits = 1.;
  double ConvertVoltageToGraphUnits = 1.;

  TLine *Trigger_Line[DGManager->GetNumChannels()];
  TBox *BaselineCalcRegion_Box[DGManager->GetNumChannels()];

  double SignalPolarity[DGManager->GetNumChannels()];
  
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
    Trigger_Line[ch] = new TLine();
    Trigger_Line[ch]->SetLineWidth(2);
    Trigger_Line[ch]->SetLineStyle(2);
    Trigger_Line[ch]->SetLineColor(ch+1);

    BaselineCalcRegion_Box[ch] = new TBox();
    BaselineCalcRegion_Box[ch]->SetFillStyle(3002);
    BaselineCalcRegion_Box[ch]->SetFillColor(ch+1);
    
    if(DGScopeChannelPosPolarity_RB[ch]->IsDown())
      SignalPolarity[ch] = 1.0;
    else
      SignalPolarity[ch] = -1.0;
  }
  double VertPosOffset, ChTrigThr;
  VertPosOffset = ChTrigThr = 0;

  double BaseCalcMin, BaseCalcMax, BaseCalcResult;
  BaseCalcMin = BaseCalcMax = BaseCalcResult = 0;

  double SampleHeight = 0.;

  uint32_t ChannelToHistogram = 0;
    
  // Bools to determine whether the incoming waveforms are filted into
  // a spectrum by pulse height (maximum value of voltage above the
  // baseline during the entire record length) or by pulse area
  // (integrated area between the baseline and the voltage trace)
  bool AnalyzePulseHeight = DGScopeSpectrumAnalysisHeight_RB->IsDown();
  bool AnalyzePulseArea = DGScopeSpectrumAnalysisArea_RB->IsDown();

  // Minimum value that will be histogrammed into the spectrum. 
  int LowerLevelDiscr = DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->GetIntNumber();
  int UpperLevelDiscr = DGScopeSpectrumAnalysisULD_NEL->GetEntry()->GetIntNumber();
  bool DiscrOKForOutput = false;

  // Variables contain the pulse height and integrated pulse area
  // values. Units are in [ADC] until transformed by calibration
  // factor into units of [keV]
  double PulseHeight = 0.;
  double PulseArea = 0.;

  // Iterate through all 8 digitizer channels
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){

    // Initialize spectrum histograms for each channel, deleting the
    // previous ones if they exist to prevent memory leaks. Assign the
    // correct graphical styles as necessary
    if(DGScopeSpectrum_H[ch]) delete DGScopeSpectrum_H[ch];
    DGScopeSpectrum_H[ch] = new TH1F("","",bins,minBin,maxBin);

    DGScopeSpectrum_H[ch]->SetFillColor(ch+1);
    DGScopeSpectrum_H[ch]->SetLineWidth(2);
    DGScopeSpectrum_H[ch]->SetLineColor(ch+1);
    DGScopeSpectrum_H[ch]->SetFillStyle(4000);
    DGScopeSpectrum_H[ch]->SetFillColor(0);
  }

  gStyle->SetOptStat("ne");

  // Variables for plotting vertical post-trigger and calibration lines
  double linePosX = 0;
  TLine *Calibration_Line;


  //////////////////////////////////////////////
  // Prepare the ROOT output file and objects //
  //////////////////////////////////////////////

  // Define a vector of vectors that will hold the digitized waveforms
  // in all channels (units of [ADC]). The outer vector (size 8)
  // represents each digitizer channel; the inner vector (size
  // RecordLength) represents the waveform. The start address of each
  // outer vector will be used to create a unique branch in the
  // waveform TTree object to store each of the 8 digitizer channels 
  vector< vector<int> > VoltageInADC_AllChannels;

  // Resize the outer and inner vector to the appropriate, fixed size
  VoltageInADC_AllChannels.resize(DGManager->GetNumChannels());
  for(int i=0; i<DGManager->GetNumChannels(); i++)
    VoltageInADC_AllChannels[i].resize(RecordLength);


  ///////////////////////////////////////////////////////
  // Program V1720 digitizer with acquisition settings //
  ///////////////////////////////////////////////////////

  // Reset the digitizer to default state
  //DGManager->FreeEvent((void **)&EventWaveform);
  //DGManager->FreeEvent(&EventWaveform);
  //DGManager->FreeReadoutBuffer(&Buffer);
  DGManager->Reset();
  
  // Set the trigger threshold individually for each of the 8
  // digitizer channels [ADC] and the DC offsets for each channel
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
    DGManager->SetChannelTriggerThreshold(ch,ChannelTriggerThreshold[ch]);
    DGManager->SetChannelDCOffset(ch, DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());
  }

  // Set the trigger mode
  switch(TriggerMode){

    // Mode: External trigger (NIM logic input signal) only
  case 0:
    DGManager->EnableExternalTrigger("NIM");
    DGManager->DisableAutoTrigger(ChannelEnableMask);
    DGManager->DisableSWTrigger();
    break;

    // Mode: External trigger (TTL logic input signal) only
  case 1:
    DGManager->EnableExternalTrigger("TTL");
    DGManager->DisableAutoTrigger(ChannelEnableMask);
    DGManager->DisableSWTrigger();
    break;
    
    // Mode: Automatic channel threshold triggering only
  case 2:
    DGManager->DisableExternalTrigger();
    DGManager->EnableAutoTrigger(ChannelEnableMask);
    DGManager->DisableSWTrigger();
    break;

    // Mode: Software trigger only
  case 3:
    DGManager->DisableExternalTrigger();
    DGManager->DisableAutoTrigger(ChannelEnableMask);
    DGManager->EnableSWTrigger();
    break;
  }
  
  // Set the record length of the acquisition window
  DGManager->SetRecordLength(RecordLength);

  // Set the channel enable mask
  DGManager->SetChannelEnableMask(ChannelEnableMask);

  // If the digitizer is to be operated in coincidence mode and "the
  // conditions are right" as Oye says, set the trigger source mask by
  // "or" bit operating  bits 24:26 of the preset trigger coincidence
  // level variable into the digitizer's trigger source enable mask
  if(TriggerCoincidenceLevel<NumDGChannelsEnabled 
     and DGScopeTriggerCoincidenceEnable_CB->IsDisabledAndSelected()){
    uint32_t TriggerSourceEnableMask = 0;
    DGManager->GetRegisterValue(0x810C,&TriggerSourceEnableMask);
    TriggerSourceEnableMask = TriggerSourceEnableMask | TriggerCoincidenceLevel_BitShifted;
    DGManager->SetRegisterValue(0x810C,TriggerSourceEnableMask);
  }
  
  // Set the maximum number of events that will be accumulated before
  // the V1720 FPGA buffer is dumped to PC memory
  DGManager->SetMaxNumEventsBLT(5);

  // Set the acquisition to be controlled via software
  DGManager->SetSWAcquisitionMode();

  // Set the percentage of acquisition window that occurs after trigger
  DGManager->SetPostTriggerSize(PostTriggerSize);

  // Allocate memory for the readout buffer
  DGManager->MallocReadoutBuffer(&Buffer, &Size);

  // Set the V1720 to begin acquiring data
  DGManager->SWStartAcquisition();


  ///////////////////////////////////////////////////
  // V1720 digitizer acquisition and data plotting //
  ///////////////////////////////////////////////////
  // The following loops reads digitized data from the digitizers into
  // local PC memory, principally as arrays of voltage versus time (or
  // sample). To maximize data throughput, the following loop should
  // be be as efficient as possible. The highest data throughput rate
  // will be achieved when neither the waveform or spectrum is plotted
  // (ie, DGScope is in "blank" mode), but an effort is made to
  // streamline the loop as much as possible
  
  // The following terminology is important:
  // V1720 buffer == the memory buffer onboard the FPGA of the V1720 board
  // PC buffer == the memory buffer allocated locally on the PC
  // Event == an acquisition window caused by a channel trigger threshold being exceeded
  // NumEvents == the number of events that is allowed to accumulate on the V1720 buffer
  //              before being automatically readout into the PC buffer
  // Record Length == the length of the acquisition window in 4 ns units
  // Sample ==  a single value between 0 and 4095 of digitized voltage
  

  // The acquisition and data plotting loop is run provided that the
  // DGScopeEnable bool is true (see ADAQAcquisitionGUI::HandleScopeButtons)

  while(DGScopeEnable){

    /////////////////////////////////////////////
    // Create separate ROOT thread for processing

    // Run the processes in a seperate thread to maintain full access
    // to all GUI functions in the main processing thread
    gSystem->ProcessEvents();


    ///////////////////////////
    // Set graphical attributes

    // Graphical attributes are set at the highest level of the loop
    // to minimize the number of contributes since the graphical
    // attributes apply to all events/all channels

    // Get the horizontal and vertical positions of the double
    // sliders that border the DGScope embedded canvas (the
    // "zoom"). The slider end values are between 0 and 1;
    // multiplying the slider values by the appropriate
    // conversion factor results in correct X and Y axes
    DGScopeHorizontalScale_THS->GetPosition(&xMin, &xMax);
    DGScopeVerticalScale_DVS->GetPosition(&yMin, &yMax);

    // Plot the X axis in sample number
    if(PlotXAxisInSamples){
      xMin *= RecordLength;
      xMax *= RecordLength;
    }
    else{ // Plot the X axis in nanoseconds
      xMin *= (RecordLength*DGManager->NanosecondsPerSample);
      xMax *= (RecordLength*DGManager->NanosecondsPerSample);
      
      ConvertTimeToGraphUnits = DGManager->NanosecondsPerSample;
    }
    
    // Plot the Y axis in analog-to-digitial conversion bits (0
    // to 4095 for the 12-bit V1720 digitizer)
    if(PlotYAxisInADC){
      yMin *= DGManager->GetMaxBit();
      yMax *= DGManager->GetMaxBit();
    }
    else{ // Plot the Y axis in millivolts
      yMin *= DGManager->GetMaxBit()*DGManager->MillivoltsPerBit;
      yMax *= DGManager->GetMaxBit()*DGManager->MillivoltsPerBit;
      
      ConvertVoltageToGraphUnits = DGManager->MillivoltsPerBit;
    }


    /////////////////////
    // Begin data readout

    // Read data from the V1720 buffer into the PC buffer
    DGManager->ReadData(Buffer, &BufferSize);    

    // Determine the number of events in the buffer
    DGManager->GetNumEvents(Buffer, BufferSize, &NumEvents);

    // For each event in the PC memory buffer...
    for(uint32_t i=0; i<NumEvents; i++){
      
      // Get the event information
      DGManager->GetEventInfo(Buffer, BufferSize, i, &EventInfo, &EventPointer);
      
      // Decode the event and obtain the waveform (voltage as a function of time)
      //DGManager->DecodeEvent(EventPointer, (void **)&EventWaveform);
      DGManager->DecodeEvent(EventPointer, &EventWaveform);

      // If there is no waveform in the PC buffer, continue in the
      // while loop to avoid segfaulting
      if(EventWaveform==NULL)
	continue;

      // For each channel...
      for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
	
	// Only proceed to waveform analysis if the channel is enabled
	if(!DGScopeChannelEnable_CB[ch]->IsDisabledAndSelected())
	  continue;
	
	// Initialize the pulse height and pulse area variables
	PulseHeight = PulseArea = 0.; // [ADC]

	// Reset all channel baseline before each event
	BaselineCalcResult[ch] = 0.; // [ADC]
	
	// For all of the samples in the acquisition window of length RecordLength...
	for(uint32_t sample=0; sample<RecordLength; sample++){
	  
	  // Get the digitized time in units of sample number
	  Time[sample] = sample; // [sample]

	  // Convert the time [sample] into suitable form for graphing
	  // (accounts for units of sample/ns)
	  Time_graph[sample] = Time[sample] * ConvertTimeToGraphUnits;
	  
	  // Get the digitized voltage in units of analog-to-digital conversion bits
	  Voltage[sample] = EventWaveform->DataChannel[ch][sample]; // [ADC]
	  VoltageInADC_AllChannels[ch][sample] = Voltage[sample];

	  // Convert the voltage [ADC] into suitable form for graphing
	  // (accounts for units of ADC/mV and vertical offset)
	  Voltage_graph[sample] = (Voltage[sample] + DGScopeVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber()) * ConvertVoltageToGraphUnits;
	  
	  // If the sample number falls within the specified time
	  // window, calculate the baseline by taking the average of
	  // all samples that fall within that time window
	  if(sample>BaselineCalcMin[ch] and sample<BaselineCalcMax[ch])
	    BaselineCalcResult[ch] += Voltage[sample]*1.0/(BaselineCalcLength[ch]-1); // [ADC]
	  
	  // For those sample of the pulse that occur after the
	  // baseline has been calculate, analyze the pulses to obtain
	  // pulse height or area spectrum
	  else if(sample>BaselineCalcMax[ch]){
	    
	    // Calculate the absolute value of the sample voltage,
	    // e.g. the height "above" or "below" the baseline for
	    // positive and negative pulses respectively
	    SampleHeight = SignalPolarity[ch] * (Voltage[sample] - BaselineCalcResult[ch]);

	    // Simple algorithm to determine maximum peak height in the pulse
	    if(SampleHeight>PulseHeight)
	      PulseHeight = SampleHeight;
	    
	    // Integrate the area under the pulse
	    if(SampleHeight>(ChannelTriggerThreshold[ch]-BaselineCalcResult[ch]))
	      PulseArea += SampleHeight;
	  }
	}
	
	// If a CalibrationManager (a ROOT TGraph) exists, ie, has
	// been successfully created and is valid for interpolation
	// then convert PulseHeight/Area
	if(UseCalibrationManager[ch]){
	  // Use the ROOT TGraph CalibrationManager to convert the
	  // pulse height/area from ADC to keV using LINEAR
	  // interpolation on the pre-assigned calibration points
	  PulseHeight = CalibrationManager[ch]->Eval(PulseHeight);
	  PulseArea = CalibrationManager[ch]->Eval(PulseArea);

	  // Use the ROOT TGraph CalibrationManager to convert the
	  // pulse height/area from ADC to keV using SPLINE
	  // interpolation on the pre-assigned calibration
	  // points. This settings is only useful if there are a large
	  // number of points and even then, is probably not that
	  // valuable. But it's here to show that more complicated
	  // interpolation can easily be accomplished with ROOT's
	  // PulseHeight =CalibrationManager->Eval(PulseHeight,0,"S"); 
	  // PulseArea = CalibrationManager->Eval(PulseHeight,0,"S");
	}
	
	// The following chunk o' code takes the analyzed waveform and
	// determined if the pulse unit should be binned in the
	// appropriate spectrum histogram. It also determines whether
	// the desired channel's waveform is within the LLD/ULD
	// window; if it is and the user has specified that the "LD
	// trigger" should be enabled, then OK this group trigger for
	// output to the ROOT file.

	// If analysis of pulse height is being performed...
        if(AnalyzePulseHeight){
	  // Determine if the pulse is within the LLD/ULD window ...
          if(PulseHeight>LowerLevelDiscr and PulseHeight<UpperLevelDiscr){
	    // ...and if so fill the PHS histogram
            DGScopeSpectrum_H[ch]->Fill(PulseHeight);
	    // Determine if this channel's waveform should be used to
	    // set the "LD trigger" ...
	    if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and
	       ch == DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->GetSelected())
	      // ... and if so then OK this group trigger (8 channel's
	      // worth of waveform data) for output later to the ROOT
	      // file after all 8 channels have been iterated over
	      DiscrOKForOutput = true;
	  }
        }
	// Same as above except for the pulse area analysis
        else if(AnalyzePulseArea){
          if(PulseArea>LowerLevelDiscr and PulseArea<UpperLevelDiscr){
            DGScopeSpectrum_H[ch]->Fill(PulseArea);
            if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and
               ch == DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->GetSelected())
              DiscrOKForOutput = true;
          }
        }


	// The TTree that holds the waveforms must have a branch
	// created for the waveforms; however, that branch holds the
	// waveforms as arrays, which requires specifying (at branch
	// creation time) the array length (I hate arrays). The array
	// length of "VoltageInADC" is set when the acquisition
	// begins, but I want the ability to be able to create/close
	// multiple ROOT files during the acquisition (provided by
	// TGTextButtons) as well as only dump data to those files
	// when desired (provided by TGCheckButton). The ROOT "slots"
	// for these three buttons are implemented in
	// ADAQAcquisitionGUI::HandleScopeButtons, which precludes creating
	// the branch there since the fixed-length array variable
	// "VoltageInADC" must be set in this member function at the
	// beginning of each acquisition to allow different
	// acquisition windows (length of that array). 
	//
	// Therefore, the solution is for the acquisition start
	// TGTextButton to trigger the "BranchWaveformtree" boolean to
	// true such that the following commands within the "if"
	// statement are called **only once per acquisition start** to
	// create the TTree branch with the correct array properties.
	if(BranchWaveformTree){
	  ostringstream ss;

	  // For each digitizer channel....
	  for(Int_t channel=0; channel<DGManager->GetNumChannels(); channel++){
	    
	    // ...create a channel-specific name string...
	    ss << "VoltageInADC_Ch" << channel;
	    string WaveformTreeBranchName = ss.str();
	    ss.str("");

	    // ...and use it to specify a channel-specific branch in
	    /// the waveform TTree The branch holds the address of the
	    // vector that contains the waveform as a function of
	    // record length and the RecordLength of each waveform
	    WaveformTree->Branch(WaveformTreeBranchName.c_str(), 
				 &VoltageInADC_AllChannels.at(channel),
				 RecordLength,
				 0);
	  }
	  BranchWaveformTree = false;
	}

	// Store the digitized waveform in the Root TTree object
	// provided that the Waveform tree exists and that data should
	// be dumped into the tree indicated by the TGCheckButton
	//if(WaveformTree and DGScopeDataStorageEnable_CB->IsDown())
	//WaveformTree->Fill();
	
	// Plot the digitized waveforms in 'oscilloscope' mode
	if(PlotWaveform){

	  // Ensure the waveforms always plot in linear-linear mode
	  // (required to plot waveforms after a spectrum is being
	  // plotted with a logarhythmic y-axis)
	  gPad->SetLogx(false);
	  gPad->SetLogy(false);
	  
	  // Ensure to free previous memory allocated to the TGraphs
	  // to prevent fairly massive memory leakage
	  if(DGScopeWaveform_G[ch]) delete DGScopeWaveform_G[ch];
	  DGScopeWaveform_G[ch] = new TGraph(RecordLength, Time_graph, Voltage_graph);

	  // At minimum, a single channel's waveform is graphed. The
	  // "lowest enabled channel", ie, the channel closest to 0
	  // that is plotted, must set the graphical attributes of the
	  // plot, including defining the X and Y axies; subsequent
	  // channel waveform graphs will then be plotted on top.

	  if(ch==LowestEnabledChannel){
	    DGScopeWaveform_G[ch]->SetLineWidth(2);
	    DGScopeWaveform_G[ch]->SetLineColor(ch+1);
	    DGScopeWaveform_G[ch]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
	    DGScopeWaveform_G[ch]->GetXaxis()->SetTitle(DGScopeDisplayXTitle_TEL->GetEntry()->GetText());
	    DGScopeWaveform_G[ch]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeWaveform_G[ch]->GetXaxis()->CenterTitle();
	    DGScopeWaveform_G[ch]->GetXaxis()->SetRangeUser(xMin, xMax);
	    DGScopeWaveform_G[ch]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
	    DGScopeWaveform_G[ch]->GetYaxis()->CenterTitle();
	    DGScopeWaveform_G[ch]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeWaveform_G[ch]->GetYaxis()->SetRangeUser(yMin,yMax);
	    DGScopeWaveform_G[ch]->Draw("ALP");

	    if(DrawLegend)
	      DGScopeWaveform_L->Draw();
	  }
	  else{
	    DGScopeWaveform_G[ch]->SetLineWidth(2);
	    DGScopeWaveform_G[ch]->SetLineColor(ch+1);
	    DGScopeWaveform_G[ch]->Draw("L");
	  }

	  // Draw a horizontal dotted line of the same color as the
	  // channel waveform representing the channel trigger
	  // threshold. Ensure accounting for channel vertical offset
	  VertPosOffset = DGScopeVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber();
	  ChTrigThr = (DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() + VertPosOffset) * ConvertVoltageToGraphUnits;

	  Trigger_Line[ch]->DrawLine(xMin, ChTrigThr, xMax, ChTrigThr);
	  
	  // Draw a shaded box region to represent the area of the
	  // waveform being used to calculate the current baseline for
	  // each digitized waveform
	  BaseCalcMin = BaselineCalcMin[ch]*ConvertTimeToGraphUnits;
	  BaseCalcMax = BaselineCalcMax[ch]*ConvertTimeToGraphUnits;
	  BaseCalcResult = (BaselineCalcResult[ch] + VertPosOffset) * ConvertVoltageToGraphUnits;
	  BaselineCalcRegion_Box[ch]->DrawBox(BaseCalcMin, (BaseCalcResult-100),
					      BaseCalcMax, (BaseCalcResult+100));
	}

	// Plot the integrated pulses in 'multichannel analyzer' mode
	else if(PlotSpectrum){

	  // Determine the channel desired for histogramming into a pulse height spectrum
	  ChannelToHistogram = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
	  
	  // Update the spectrum every N events, where N is determined
	  // // via the DGScopeSpectrumRefreshRate_NBL widget.
	  if(int(DGScopeSpectrum_H[ChannelToHistogram]->GetEntries())%DGScopeSpectrumRefreshRate_NEL->GetEntry()->GetIntNumber()==0){
	    
	    // Get the positions of the sliders that bound the DGScope
	    // embedded canvas and convert them into the appropriate
	    // units for zooming on the spectrum
	    DGScopeHorizontalScale_THS->GetPosition(&xMin, &xMax);
	    DGScopeVerticalScale_DVS->GetPosition(&yMin, &yMax);
	    
	    xMin *= maxBin;
	    xMax *= maxBin;

	    yMin *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin());
	    yMax *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin())*1.1;
	    
	    // Enable the X and Y axes to be plotted on a log. scale
	    if(DGScopeSpectrumXAxisLog_CB->IsDown())
	      DGScope_EC->GetCanvas()->SetLogx(true);
	    else
	      DGScope_EC->GetCanvas()->SetLogx(false);
	    
	    if(DGScopeSpectrumYAxisLog_CB->IsDown()){
	      DGScope_EC->GetCanvas()->SetLogy();
	      yMin += 1;
	    }
	    else
	      DGScope_EC->GetCanvas()->SetLogy(false);

	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitle(DGScopeDisplayXTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleSize(DGScopeDisplayXTitleSize_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetRangeUser(xMin,xMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleSize(DGScopeDisplayYTitleSize_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetRangeUser(yMin,yMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
	    if(DrawLegend)
	      DGScopeSpectrum_H[ChannelToHistogram]->SetStats(false);
	    DGScopeSpectrum_H[ChannelToHistogram]->Draw("L");
	    
	    // If the calibration mode is enabled, then draw the third
	    // slider position from the horizontal triple slider and
	    // use its position to calculate the correct calibration
	    // factor. Update the calibration factor number entry
	    // widget with the calculated value
	    if(DGScopeSpectrumCalibration_CB->IsDown()){
	      linePosX = DGScopeHorizontalScale_THS->GetPointerPosition()*maxBin;
	      Calibration_Line = new TLine(linePosX, yMin, linePosX, yMax);
	      Calibration_Line->SetLineColor(2);
	      Calibration_Line->SetLineStyle(2);
	      Calibration_Line->SetLineWidth(2);
	      Calibration_Line->Draw();

	      // Use the pointer to position to update the pulse unit
	      // NEL widget to aid the user to determining the exact
	      // position desired calibration features in the PAS/PHS
	      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(DGScopeHorizontalScale_THS->GetPointerPosition() * maxBin);
	    }
	    DGScope_EC->GetCanvas()->Update();
	  }
	}
      
	// Do not plot anything. This mode is most useful to
	// maximizing the data throughput rate.
	else if(PlotBlank){
	}
      }

      // After all the channels in the event have been iterated
      // through to extract the waveforms, store the digitized
      // waveforms for all channels in the ROOT TTree object provided
      // that 
      // 0. the waveform tree has been created 
      // 1. the user wants to dump the data to a ROOT file
      if(WaveformTree and DGScopeDataStorageEnable_CB->IsDown()){

        // If the user has specified that a specific channel's
        // waveform falling with the LLD/ULD window should be used as
        // the "trigger" for plotting the PAS/PHS and writing to a
        // ROOT file BUT the present waveform is NOT within the
        // LLD/ULD window (indicated by the DiscrOKForOutput bool set
        // above during analysis of the readout waveform) then do NOT
        // write the waveform to the ROOT TTree. This is effectively
        // triggering the digitizer on a specific pulse/energy window
        // of a waveform on a desired channel
        if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and !DiscrOKForOutput)
          continue;

        // Fill the TTree
        WaveformTree->Fill();

        // Reset the bool used to determine if the LLD/ULD window
        // should be used as the "trigger" for writing waveforms
	DiscrOKForOutput = false;
      }
      if(PlotWaveform)
	DGScope_EC->GetCanvas()->Update();
      
      // Free the memory allocated to the V1720 event buffer
      //DGManager->FreeEvent((void **)&EventWaveform);
      DGManager->FreeEvent(&EventWaveform);
    }
  }
  // Free the memory allocated to the PC readout buffer
  DGManager->FreeReadoutBuffer(&Buffer);
}


// The mandatory C++ main function
int main(int argc, char **argv)
{
  // Create a standalone application that runs outside of a ROOT seesion
  TApplication *TheApplication = new TApplication("ADAQAcquisitionGUI", &argc, argv);
  
  // Create variables for width and height of the top-level GUI window
  int Width = 1115;
  int Height = 825;

  // If the user specifies "small" for the first command line
  // arguments then change the width and height settings
  if(argc==2){
    string arg1 = argv[1];
    if(arg1 == "small"){
      Width = 980;
      Height = 650;
    }
    else
      cout << "Error! Only argument allowed is 'small'!\n" << endl;
  }
  
  // Create an object of type ADAQAcquisitionGUI and connect its "CloseWindow" function
  ADAQAcquisitionGUI *MainFrame = new ADAQAcquisitionGUI(Width, Height);
  MainFrame->Connect("CloseWindow()", "ADAQAcquisitionGUI", MainFrame, "HandleDisconnectAndTerminate(bool)");
  
  // Run the standalone application
  TheApplication->Run();
  
  // Clean up memory upon completion
  delete MainFrame;
  
  return 0;
}