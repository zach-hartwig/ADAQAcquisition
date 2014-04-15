// ROOT 
#include <TTree.h>
#include <TFile.h>
#include <TGText.h>
#include <TGTextView.h>
#include <TGFileDialog.h>

// C++ 
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <bitset>
using namespace std;

// Boost
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

// CAEN 
#include "CAENDigitizer.h"

// ADAQ 
#include "ADAQRootClasses.hh"
#include "ADAQHighVoltage.hh"
#include "ADAQDigitizer.hh"
#include "ADAQBridge.hh"

#include "ADAQAcquisition.hh"
#include "ADAQEnumerators.hh"
#include "Version.hh"


ADAQAcquisition::ADAQAcquisition(int W, int H)
  : TGMainFrame(gClient->GetRoot()),
    DisplayWidth(W), DisplayHeight(H), 
    V1718Enable(true),
    V1720Enable(true), V1720BoardAddress(0x00420000),
    V6534Enable(true), V6534BoardAddress(0x42420000),
    VMEConnectionEstablished(false),
    HVMonitorEnable(false), DGScopeEnable(false),
    NumDataChannels(8), BuildInDebugMode(false),
    AcquisitionTimerEnabled(false), 
    AcquisitionTime_Start(0.), AcquisitionTime_Stop(0.),
    DataFileName("DefaultData"), DataFileExtension(".adaq"),
    SpectrumFileName("DefaultSpectrum"), SpectrumFileExtension(".dat"),
    GraphicsFileName("DefaultGraphics"), GraphicsFileExtension(".eps"),
    ColorManager(new TColor), RNG(new TRandom)
{
  // Hierarchical cleanup of all child frames during destructor
  SetCleanup(kDeepCleanup);

  /////////////////////////////
  // Initialize ADAQ classes //
  /////////////////////////////
  // Create "managers" for the V6534 high voltage and V1720 digitizer
  // boards that will be used to provide information to this class as
  // well as full control over each of the boards. Note that
  // ADAQAcquisition can be run without either board by setting the
  // appropriate V*Enable boolean variables above
  HVManager = new ADAQHighVoltage;
  HVManager->SetVerbose(true);
  
  DGManager = new ADAQDigitizer;
  DGManager->SetVerbose(true);

  BRManager = new ADAQBridge;
  BRManager->SetVerbose(true);
  
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
  // DGScopeWaveform_Leg ROOT legend object
  string DGScopeWaveformTitle[8] = {"Ch 0", "Ch 1", "Ch 2", "Ch 3", 
				    "Ch 4", "Ch 5", "Ch 6", "Ch 7"};
  
  // Create a ROOT legend for the waveform graph
  DGScopeWaveform_Leg = new TLegend(0.91, 0.5, 0.99, 0.95);
  
  // For each channel on the digitizer, create the appropriate label
  // and symbol in the ROOT legend using a dummy TGraph object to set
  // the line attributes. Also, initialize the TH1F objects
  // representing the pulse heigh spectrum for each channel
  for(int i=0; i<NumDataChannels; i++){
    TGraph *Dummy_G = new TGraph();
    Dummy_G->SetLineColor(i+1);
    Dummy_G->SetLineWidth(4);

    assert(i<9);

    DGScopeWaveform_Leg->AddEntry(Dummy_G, DGScopeWaveformTitle[i].c_str(), "L");
    DGScopeWaveform_Leg->SetFillColor(18);
    DGScopeWaveform_Leg->SetTextSize(0.04);

    DGScopeChannelTrigger_L[i] = new TLine;
    DGScopeChannelTrigger_L[i]->SetLineColor(i+1);
    DGScopeChannelTrigger_L[i]->SetLineWidth(2);
    DGScopeChannelTrigger_L[i]->SetLineStyle(7);
    
    DGScopeBaselineCalcRegion_B[i] = new TBox;
    DGScopeBaselineCalcRegion_B[i]->SetFillColor(i+1);
    DGScopeBaselineCalcRegion_B[i]->SetFillStyle(3001);
  }

  DGScopeSpectrumCalibration_L = new TLine;
  DGScopeSpectrumCalibration_L->SetLineColor(kRed);
  DGScopeSpectrumCalibration_L->SetLineWidth(2);
  DGScopeSpectrumCalibration_L->SetLineStyle(7);
  

  // Create a dummy TLine object to add a single entry to thewaveform
  // graph legend representing the trigger lines
  TLine *Dummy_Line = new TLine();
  Dummy_Line->SetLineColor(4);
  Dummy_Line->SetLineStyle(2);
  Dummy_Line->SetLineWidth(4);
  DGScopeWaveform_Leg->AddEntry(Dummy_Line, "Trig", "L");
  
  for(int ch=0; ch<NumDataChannels; ch++){
    DGScopeWaveform_G[ch] = new TGraph;
    DGScopeSpectrum_H[ch] = new TH1F;

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
  FillRegisterFrame();
  FillPulserFrame();
  FillVoltageFrame();
  FillScopeFrame();


  /////////////////////////////////////////////////
  // Init. and map the GUI windows, set defaults //
  /////////////////////////////////////////////////

  string TitleString;
  if(VersionString == "Development")
    TitleString = "AIMS Data Acquisition (Development version)               Fear is the mind-killer.";
  else
    TitleString = "AIMS Data Acquisition (Production version " + VersionString + ")               Fear is the mind-killer.";

  SetWindowName(TitleString.c_str());
  Resize(DisplayWidth, DisplayHeight);
  MapSubwindows();
  MapWindow();
}


ADAQAcquisition::~ADAQAcquisition()
{
  for(int ch=0; ch<NumDataChannels; ch++){
    delete CalibrationManager[ch];
    delete DGScopeSpectrum_H[ch];
    delete DGScopeWaveform_G[ch];
    delete DGScopeBaselineCalcRegion_B[ch];
    delete DGScopeChannelTrigger_L[ch];
  }

  delete DGScopeWaveform_Leg;

  delete DGManager;
  delete HVManager;
}


void ADAQAcquisition::CreateTopLevelFrames()
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

  ConnectionTab = TopLevelTabs->AddTab(" VME Connection ");
  ConnectionFrame = new TGCompositeFrame(ConnectionTab, 60, 20, kVerticalFrame);
  ConnectionTab->AddFrame(ConnectionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  RegisterTab = TopLevelTabs->AddTab(" Register R/W ");
  RegisterFrame = new TGCompositeFrame(RegisterTab, 200, 20, kVerticalFrame);
  RegisterTab->AddFrame(RegisterFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  PulserTab = TopLevelTabs->AddTab(" Pulsers ");
  PulserFrame = new TGCompositeFrame(PulserTab, 60, 20, kVerticalFrame);
  PulserTab->AddFrame(PulserFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  VoltageTab = TopLevelTabs->AddTab(" High Voltage ");
  VoltageFrame = new TGCompositeFrame(VoltageTab, 60, 20, kHorizontalFrame);
  VoltageTab->AddFrame(VoltageFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  ScopeTab = TopLevelTabs->AddTab(" Acquisition ");
  ScopeFrame = new TGCompositeFrame(ScopeTab, 60, 20, kHorizontalFrame);
  ScopeTab->AddFrame(ScopeFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  TabFrame->AddFrame(TopLevelTabs, new TGLayoutHints(kLHintsTop, 5,5,5,5));


  ////////////////////////////////////////////
  // Add top level frames to the main frame //
  ////////////////////////////////////////////
  TopFrame->AddFrame(TabFrame, new TGLayoutHints(kLHintsTop, 5,5,5,5));

  AddFrame(TopFrame, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
}


// The "ConnectionFrame" holds ROOT widgets that control aspects of
// connecting to the VME-USB interface, providing connection,
// initialization and read/write control to registers of V1720
// digitizer, V6534 high voltage, and V1718 USB-VME boards (planned).
void ADAQAcquisition::FillConnectionFrame()
{
  /////////////////////////////
  // The main connection bar //
  /////////////////////////////

  TGGroupFrame *Connection_GF = new TGGroupFrame(ConnectionFrame,"Initiate VME Connection", kVerticalFrame);
  Connection_GF->SetTitlePos(TGGroupFrame::kCenter);

  // ROOT text button that controls connection of ADAQAcquisition to the VME boards
  Connection_GF->AddFrame(V1718Connect_TB = new TGTextButton(Connection_GF, "Disconnected: click to connect", V1718Connect_TB_ID),
			    new TGLayoutHints(kLHintsExpandX, 5,5,25,5));
  V1718Connect_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleConnectionButtons()");
  V1718Connect_TB->Resize(500,40);
  V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  V1718Connect_TB->ChangeOptions(V1718Connect_TB->GetOptions() | kFixedSize);

  Connection_GF->AddFrame(ConnectionOutput_TV = new TGTextView(Connection_GF, 700, 400, -42),
			  new TGLayoutHints(kLHintsTop | kLHintsExpandX, 15,15,5,25));
  ConnectionOutput_TV->SetBackground(ColorManager->Number2Pixel(18));
  
  ConnectionFrame->AddFrame(Connection_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));


  //////////////////////////////////////
  // The address/board-enable widgets //
  //////////////////////////////////////

  TGGroupFrame *ModuleSettings_GF = new TGGroupFrame(ConnectionFrame, "VME Module Settings", kHorizontalFrame);
  ModuleSettings_GF->SetTitlePos(TGGroupFrame::kCenter);

  vector<string> AddressTitle;
  AddressTitle += "", "V1720 base address", "V6534 base address";

  vector<int> BoardEnableID, BoardAddressID, BoardAddress;
  BoardEnableID += (int)0, (int)V1720BoardEnable_TB_ID, (int)V6534BoardEnable_TB_ID;
  BoardAddressID += (int)0, (int)V1720BoardAddress_ID, (int)V6534BoardAddress_ID;
  BoardAddress += (int)0, (int)V1720BoardAddress, (int)V6534BoardAddress;

  for(int board=0; board<3; board++){
    
    // The V1718 board has no address/enable functionality. Insert a
    // placeholder since the standard for vectors holding widgets for
    // each board is to have three in order
    if(board == 0){
      BoardAddress_NEF.push_back(0);
      BoardEnable_TB.push_back(0);
      continue;
    }

    TGVerticalFrame *BoardAddress_VF = new TGVerticalFrame(ModuleSettings_GF);
    BoardAddress_VF->AddFrame(new TGLabel(BoardAddress_VF, AddressTitle[board].c_str()), new TGLayoutHints(kLHintsCenterX, 5,5,5,0));
    
    TGHorizontalFrame *BoardAddress_HF = new TGHorizontalFrame(BoardAddress_VF);
    BoardAddress_HF->AddFrame(new TGLabel(BoardAddress_HF,"0x"), new TGLayoutHints(kLHintsExpandY, 5,0,0,5));
    
    BoardAddress_NEF.push_back(new TGNumberEntryField(BoardAddress_HF, BoardAddressID[board], 0,
						      TGNumberFormat::kNESHex, 
						      TGNumberFormat::kNEAPositive));
    BoardAddress_NEF[board]->SetHexNumber(BoardAddress[board]);
    BoardAddress_NEF[board]->Resize(80,20);
    BoardAddress_HF->AddFrame(BoardAddress_NEF[board], new TGLayoutHints(kLHintsExpandY, 5, 5, 0, 5));

    BoardAddress_VF->AddFrame(BoardAddress_HF,new TGLayoutHints(kLHintsExpandY, 5,5,5,5));  

    BoardEnable_TB.push_back(new TGTextButton(BoardAddress_VF, "Board enabled", BoardEnableID[board]));
    BoardEnable_TB[board]->Connect("Clicked()","ADAQAcquisition",this,"HandleConnectionButtons()");
    BoardEnable_TB[board]->Resize(110,25);
    BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(8));
    BoardEnable_TB[board]->ChangeOptions(BoardEnable_TB[board]->GetOptions() | kFixedSize);
    BoardAddress_VF->AddFrame(BoardEnable_TB[board], new TGLayoutHints(kLHintsCenterX));

    int borderLeft = 30;
    int borderRight = 30;
    if(board == 1){
      borderLeft = 30;
      borderRight = 30;
    }
    ModuleSettings_GF->AddFrame(BoardAddress_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, borderLeft,borderRight,5,5));
  }

  ConnectionFrame->AddFrame(ModuleSettings_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));
  
}


void ADAQAcquisition::FillRegisterFrame()
{
  const int NumVMEBoards = 3;

  string FrameTitle[NumVMEBoards] = {"V1718 VME/USB Module", "V1720 Digitizer Module", "V6534 High Voltage Module"};

  int ReadAddressID[NumVMEBoards] = {V1718ReadAddress_ID, V1720ReadAddress_ID, V6534ReadAddress_ID};
  int ReadValueID[NumVMEBoards] = {V1718ReadValue_ID, V1720ReadValue_ID, V6534ReadValue_ID};

  int WriteAddressID[NumVMEBoards] = {V1718WriteAddress_ID, V1720WriteAddress_ID, V6534WriteAddress_ID};
  int WriteValueID[NumVMEBoards] = {V1718WriteValue_ID, V1720WriteValue_ID, V6534WriteValue_ID};

  int ReadID[NumVMEBoards] = {V1718Read_ID, V1720Read_ID, V6534Read_ID};
  int WriteID[NumVMEBoards] = {V1718Write_ID, V1720Write_ID, V6534Write_ID};

  const int RWButtonX = 250;
  const int RWButtonY = 30;
  const int RWFGColor = ColorManager->Number2Pixel(0);
  const int RWBGColor = ColorManager->Number2Pixel(36);

  for(int board=0; board<NumVMEBoards; board++){

    ////////////////////////////////////////////////////
    // Create the group frame to hold all the subwidgets
    TGGroupFrame *RegisterRW_GF = new TGGroupFrame(RegisterFrame, FrameTitle[board].c_str(), kHorizontalFrame);
    RegisterRW_GF->SetTitlePos(TGGroupFrame::kCenter);


    /////////////////////////////////////////////////
    // The register read/write and display widgets //
    /////////////////////////////////////////////////

    TGGroupFrame *ReadCycle_GF = new TGGroupFrame(RegisterRW_GF, "Read cycle", kVerticalFrame);
    
    TGHorizontalFrame *ReadCycleAddress_HF = new TGHorizontalFrame(ReadCycle_GF);
    TGLabel *ReadCycle_L1 = new TGLabel(ReadCycleAddress_HF, "Offset address  0x");
    ReadCycle_L1->Resize(130,20);
    ReadCycle_L1->SetTextJustify(kTextRight);
    ReadCycle_L1->ChangeOptions(ReadCycle_L1->GetOptions() | kFixedSize);
    
    ReadCycleAddress_HF->AddFrame(ReadCycle_L1, new TGLayoutHints(kLHintsLeft, 5,0,5,5));

    // ROOT number entry field for setting the V6534 register address to read from
    ReadAddress_NEF.push_back(new TGNumberEntryField(ReadCycleAddress_HF, ReadAddressID[board], 0, 
						     TGNumberFormat::kNESHex,
						     TGNumberFormat::kNEAPositive));
    ReadAddress_NEF[board]->Resize(80,20);
    ReadCycleAddress_HF->AddFrame(ReadAddress_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

    // Create two outputs (hex and binary) for the register values

    TGHorizontalFrame *ReadCycleValue_HF1 = new TGHorizontalFrame(ReadCycle_GF);
    
    TGLabel *ReadCycle_L2 = new TGLabel(ReadCycleValue_HF1,"   Value  0x");
    ReadCycle_L2->Resize(130,20);
    ReadCycle_L2->SetTextJustify(kTextRight);
    ReadCycle_L2->ChangeOptions(ReadCycle_L1->GetOptions() | kFixedSize);
    ReadCycleValue_HF1->AddFrame(ReadCycle_L2, new TGLayoutHints(kLHintsLeft, 5,0,5,0));
    
    // ROOT number entry field for displaying the value from the read register address
    ReadValueHex_NEF.push_back(new TGNumberEntryField(ReadCycleValue_HF1, ReadValueID[board], 0, 
						      TGNumberFormat::kNESHex,
						      TGNumberFormat::kNEAPositive));
    ReadValueHex_NEF[board]->Resize(80,20);
    ReadValueHex_NEF[board]->SetState(false);
    ReadCycleValue_HF1->AddFrame(ReadValueHex_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,5,0));


    TGHorizontalFrame *ReadCycleValue_HF2 = new TGHorizontalFrame(ReadCycle_GF);
    
    TGLabel *ReadCycle_L3 = new TGLabel(ReadCycleValue_HF2,"          0b");
    ReadCycle_L3->Resize(130,20);
    ReadCycle_L3->SetTextJustify(kTextRight);
    ReadCycle_L3->ChangeOptions(ReadCycle_L1->GetOptions() | kFixedSize);
    ReadCycleValue_HF2->AddFrame(ReadCycle_L3, new TGLayoutHints(kLHintsLeft, 5,0,0,5));
    
    // ROOT number entry field for displaying the value from the read register address
    ReadValueBinary_TE.push_back(new TGTextEntry(ReadCycleValue_HF2, "0000 0000 0000 0000 0000 0000 0000 0000"));
    ReadValueBinary_TE[board]->Resize(250,20);
    ReadValueBinary_TE[board]->SetBackgroundColor(ColorManager->Number2Pixel(18));
    ReadCycleValue_HF2->AddFrame(ReadValueBinary_TE[board], new TGLayoutHints(kLHintsLeft, 5,5,0,5));

    ReadCycle_GF->AddFrame(ReadCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5,5,5,5));
    ReadCycle_GF->AddFrame(ReadCycleValue_HF1, new TGLayoutHints(kLHintsExpandY, 5,5,5,0));
    ReadCycle_GF->AddFrame(ReadCycleValue_HF2, new TGLayoutHints(kLHintsExpandY, 5,5,0,5));

    Read_TB.push_back(new TGTextButton(ReadCycle_GF, "VME Read", ReadID[board]));
    Read_TB[board]->Connect("Clicked()","ADAQAcquisition",this,"HandleRegisterButtons()");
    Read_TB[board]->Resize(RWButtonX, RWButtonY);
    Read_TB[board]->SetForegroundColor(RWFGColor);
    Read_TB[board]->SetBackgroundColor(RWBGColor);
    Read_TB[board]->ChangeOptions(Read_TB[board]->GetOptions() | kFixedSize);
    ReadCycle_GF->AddFrame(Read_TB[board], new TGLayoutHints(kLHintsCenterX, 5,5,5,5));
    
    // Add the read cycle group frame to the hierarchy
    RegisterRW_GF->AddFrame(ReadCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

    TGGroupFrame *WriteCycle_GF = new TGGroupFrame(RegisterRW_GF, "Write cycle", kVerticalFrame);
  
    TGHorizontalFrame *WriteCycleAddress_HF = new TGHorizontalFrame(WriteCycle_GF);

    TGLabel *WriteCycle_L1 = new TGLabel(WriteCycleAddress_HF, "Offset Address  0x");
    WriteCycle_L1->Resize(130,20);
    WriteCycle_L1->SetTextJustify(kTextRight);
    WriteCycle_L1->ChangeOptions(WriteCycle_L1->GetOptions() | kFixedSize);
    
    WriteCycleAddress_HF->AddFrame(WriteCycle_L1, new TGLayoutHints(kLHintsLeft, 5,0,5,5));
    
    // ROOT number entry field for setting the V6534 register address to write to
    WriteAddress_NEF.push_back(new TGNumberEntryField(WriteCycleAddress_HF, WriteAddressID[board], 0, 
						      TGNumberFormat::kNESHex, 
						      TGNumberFormat::kNEAPositive));
    WriteAddress_NEF[board]->Resize(80,20);

    WriteCycleAddress_HF->AddFrame(WriteAddress_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

    TGHorizontalFrame *WriteCycleValue_HF = new TGHorizontalFrame(WriteCycle_GF);
    
    TGLabel *WriteCycle_L2 = new TGLabel(WriteCycleValue_HF,"   Value  0x");
    WriteCycle_L2->Resize(130,20);
    WriteCycle_L2->SetTextJustify(kTextRight);
    WriteCycle_L2->ChangeOptions(WriteCycle_L2->GetOptions() | kFixedSize);

    WriteCycleValue_HF->AddFrame(WriteCycle_L2, new TGLayoutHints(kLHintsLeft, 5,0,5,25));

    // ROOT number entry field for setting the value that will be written to the set write register address
    WriteValue_NEF.push_back(new TGNumberEntryField(WriteCycleValue_HF, WriteValueID[board], 0, 
						    TGNumberFormat::kNESHex, 
						    TGNumberFormat::kNEAPositive));
    WriteValue_NEF[board]->Resize(80,20);

    WriteCycleValue_HF->AddFrame(WriteValue_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,5,5));

    WriteCycle_GF->AddFrame(WriteCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5,5,5,5));
    WriteCycle_GF->AddFrame(WriteCycleValue_HF, new TGLayoutHints(kLHintsExpandY, 5,5,5,5));

    Write_TB.push_back(new TGTextButton(WriteCycle_GF, "VME Write", WriteID[board]));
    Write_TB[board]->Connect("Clicked()","ADAQAcquisition",this,"HandleRegisterButtons()");
    Write_TB[board]->Resize(RWButtonX, RWButtonY);
    Write_TB[board]->SetForegroundColor(RWFGColor);
    Write_TB[board]->SetBackgroundColor(RWBGColor);
    Write_TB[board]->ChangeOptions(Write_TB[board]->GetOptions() | kFixedSize);
    WriteCycle_GF->AddFrame(Write_TB[board], new TGLayoutHints(kLHintsCenterX, 5,5,5,5));

    // Add the write cycle group frame to the hierarchy
    RegisterRW_GF->AddFrame(WriteCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

    // Add the top-level group frame to the hierarchy
    RegisterFrame->AddFrame(RegisterRW_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,40));
  }
}


void ADAQAcquisition::FillPulserFrame()
{
  const int XSize = 125;
  const int YSize = 20;

  const int NumPulsers = 2;

  string PulserTitle[NumPulsers] = {"V1718 Pulser A", "V1718 Pulser B"};

  int V1718PulserLineOut[NumPulsers] = {0,1};

  int V1718PulserID[NumPulsers] = {V1718PulserA_TB_ID, V1718PulserB_TB_ID};

  for(int pulser=0; pulser<NumPulsers; pulser++){
    
    TGGroupFrame *Pulser_GF = new TGGroupFrame(PulserFrame, PulserTitle[pulser].c_str(), kHorizontalFrame);
    Pulser_GF->SetTitlePos(TGGroupFrame::kCenter);
    PulserFrame->AddFrame(Pulser_GF, new TGLayoutHints(kLHintsCenterX, 5,5,50,30));
    
    TGVerticalFrame *PulserSettings_VF = new TGVerticalFrame(Pulser_GF);
    Pulser_GF->AddFrame(PulserSettings_VF, new TGLayoutHints(kLHintsLeft, 5,45,5,5));

    PulserSettings_VF->AddFrame(V1718PulserTimeUnit_CBL[pulser] = new ADAQComboBoxWithLabel(PulserSettings_VF, "Time unit", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->AddEntry("25 ns", 0);
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->AddEntry("1600 ns", 1);
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->AddEntry("410 us", 2);
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->AddEntry("104 ms", 3);
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserTimeUnit_CBL[pulser]->GetComboBox()->Select(0);

    PulserSettings_VF->AddFrame(V1718PulserPeriod_NEL[pulser] = new ADAQNumberEntryWithLabel(PulserSettings_VF, "Period (number of time units)", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserPeriod_NEL[pulser]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    V1718PulserPeriod_NEL[pulser]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    V1718PulserPeriod_NEL[pulser]->GetEntry()->Resize(XSize,YSize);
    V1718PulserPeriod_NEL[pulser]->GetEntry()->SetNumber(4);
  
    PulserSettings_VF->AddFrame(V1718PulserWidth_NEL[pulser] = new ADAQNumberEntryWithLabel(PulserSettings_VF, "Width (number of time units)", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    V1718PulserWidth_NEL[pulser]->GetEntry()->Resize(XSize,YSize);
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumber(1);

    PulserSettings_VF->AddFrame(V1718PulserPulses_NEL[pulser] = new ADAQNumberEntryWithLabel(PulserSettings_VF, "Number pulses (0 = infinite)", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserPulses_NEL[pulser]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    V1718PulserPulses_NEL[pulser]->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
    V1718PulserPulses_NEL[pulser]->GetEntry()->Resize(XSize,YSize);
    V1718PulserPulses_NEL[pulser]->GetEntry()->SetNumber(0);

    PulserSettings_VF->AddFrame(V1718PulserStartSource_CBL[pulser] = new ADAQComboBoxWithLabel(PulserSettings_VF, "Start source", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("Manual", 0);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("Input 1", 1);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("Input 2", 2);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("Coincidence", 3);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("VME bus", 4);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->AddEntry("Misc. signals", 6);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserStartSource_CBL[pulser]->GetComboBox()->Select(0);

    PulserSettings_VF->AddFrame(V1718PulserStopSource_CBL[pulser] = new ADAQComboBoxWithLabel(PulserSettings_VF, "Stop source", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("Manual", 0);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("Input 1", 1);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("Input 2", 2);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("Coincidence", 3);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("VME bus", 4);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->AddEntry("Misc. signals", 6);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserStopSource_CBL[pulser]->GetComboBox()->Select(0);
  
    TGVerticalFrame *PulserOutput_VF = new TGVerticalFrame(Pulser_GF);
    Pulser_GF->AddFrame(PulserOutput_VF, new TGLayoutHints(kLHintsLeft, 5,5,5,5));

    PulserOutput_VF->AddFrame(V1718PulserOutputLine_CBL[pulser] = new ADAQComboBoxWithLabel(PulserOutput_VF, "Output line", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->AddEntry("Line 0", 0);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->AddEntry("Line 1", 1);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->AddEntry("Line 2", 2);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->AddEntry("Line 3", 3);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->AddEntry("Line 4", 4);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserOutputLine_CBL[pulser]->GetComboBox()->Select(V1718PulserLineOut[pulser]);

    PulserOutput_VF->AddFrame(V1718PulserOutputPolarity_CBL[pulser] = new ADAQComboBoxWithLabel(PulserOutput_VF, "Output polarity", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserOutputPolarity_CBL[pulser]->GetComboBox()->AddEntry("Normal", 0);
    V1718PulserOutputPolarity_CBL[pulser]->GetComboBox()->AddEntry("Inverted", 1);
    V1718PulserOutputPolarity_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserOutputPolarity_CBL[pulser]->GetComboBox()->Select(0);

    PulserOutput_VF->AddFrame(V1718PulserLEDPolarity_CBL[pulser] = new ADAQComboBoxWithLabel(PulserOutput_VF, "LED polarity", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserLEDPolarity_CBL[pulser]->GetComboBox()->AddEntry("High", 0);
    V1718PulserLEDPolarity_CBL[pulser]->GetComboBox()->AddEntry("Low", 1);
    V1718PulserLEDPolarity_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserLEDPolarity_CBL[pulser]->GetComboBox()->Select(0);

    PulserOutput_VF->AddFrame(V1718PulserSource_CBL[pulser] = new ADAQComboBoxWithLabel(PulserOutput_VF, "Output source", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("Manual", 0);
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("Input 1", 1);
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("Input 2", 2);
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("Coincidence", 3);
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("VME bus", 4);
    V1718PulserSource_CBL[pulser]->GetComboBox()->AddEntry("Misc. signals", 6);
    V1718PulserSource_CBL[pulser]->GetComboBox()->Resize(XSize,YSize);
    V1718PulserSource_CBL[pulser]->GetComboBox()->Select(6);

    PulserOutput_VF->AddFrame(V1718PulserStartStop_TB[pulser] = new TGTextButton(PulserOutput_VF, "Stopped", V1718PulserID[pulser]),
			      new TGLayoutHints(kLHintsNormal, 5,5,20,5));
    V1718PulserStartStop_TB[pulser]->SetBackgroundColor(ColorManager->Number2Pixel(kRed));
    V1718PulserStartStop_TB[pulser]->Resize(200,40);
    V1718PulserStartStop_TB[pulser]->ChangeOptions(V1718PulserStartStop_TB[pulser]->GetOptions() | kFixedSize);
    V1718PulserStartStop_TB[pulser]->Connect("Pressed()","ADAQAcquisition",this,"HandlePulserButtons()");
  }  

}


// The "VoltageFrame" holds ROOT widgets for complete control of the
// HV supply board, including real-time monitoring of each channel's
// active voltage and drawn current. Setting the voltage and current
// for an individual channel are disabled while the channel power is
// on. This may be updated in the future to enable real-time changes.
void ADAQAcquisition::FillVoltageFrame()
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
    HVChannelPower_TB[ch]->Connect("Pressed()","ADAQAcquisition",this,"HandleVoltageButtons()");
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
  HVMonitorEnable_CB->Connect("Clicked()", "ADAQAcquisition", this, "HandleVoltageButtons()");
  HVMonitorEnable_CB->SetState(kButtonUp);
  
  VoltageFrame->AddFrame(HVChannelControls_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
}


void ADAQAcquisition::FillScopeFrame()
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
  ScopeFrame->AddFrame(DGScopeChannelControls_C, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  
  TGVerticalFrame *DGScopeChannelControls_VF = new TGVerticalFrame(DGScopeChannelControls_C->GetViewPort(),10,10);
  DGScopeChannelControls_C->SetContainer(DGScopeChannelControls_VF);

  // The widgets and layouts for control of channel-relevant
  // parameters for each of the 8 digitizers is identical although the
  // underlying functionality must correctly identify and set values
  // for each channel correctly. The widgets and layouts are therefore
  // constructed in a "for" loop previously initialized using
  // vectors to assign parameters that are unique to each channel
  // (names and IDs).
  for(int ch=0; ch<NumDataChannels; ch++){

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
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)","ADAQAcquisition",this,"HandleScopeNumberEntries()");
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(2000);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Resize(65,20);
    
    // ADAQ number entry to set minimum sample for baseline calculation [sample]
    DGScopeChannelControl_GF->AddFrame(DGScopeBaselineCalcMin_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Baseline min. (sample)", DGScopeChBaselineCalcMin_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)","ADAQAcquisition",this,"HandleScopeNumberEntries()");
    
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->SetNumber(10);
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->Resize(65,20);

    // ADAQ number entry to set maximum sample for baseline calculation [sample]
    DGScopeChannelControl_GF->AddFrame(DGScopeBaselineCalcMax_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Baseline max. (sample)", DGScopeChBaselineCalcMax_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->SetNumber(45);
    DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->Resize(65,20);

    DGScopeChannelControl_GF->AddFrame(DGScopeZSThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "ZS threshold", -1),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeZSThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeZSThreshold_NEL[ch]->GetEntry()->SetNumber(0);
    DGScopeZSThreshold_NEL[ch]->GetEntry()->Resize(65,20);

    DGScopeChannelControl_GF->AddFrame(DGScopeZSSamples_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "ZS samples", -1),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeZSSamples_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeZSSamples_NEL[ch]->GetEntry()->SetNumber(0);
    DGScopeZSSamples_NEL[ch]->GetEntry()->Resize(65,20);
    
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
  
  TGGroupFrame *DGScopeDisplay_GF = new TGGroupFrame(DGScopeDisplayAndControls_VF, "Multipurpose Display", kVerticalFrame);
  DGScopeDisplay_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  TGHorizontalFrame *DGScopeDisplayAndSlider_HF = new TGHorizontalFrame(DGScopeDisplay_GF);
  DGScopeDisplay_GF->AddFrame(DGScopeDisplayAndSlider_HF, new TGLayoutHints(kLHintsNormal,0,0,5,0));

  // ROOT double slider for control of the min/max of vertical axis, ie, zoom
  DGScopeDisplayAndSlider_HF->AddFrame(DGScopeVerticalScale_DVS = new TGDoubleVSlider(DGScopeDisplayAndSlider_HF, 400, kDoubleScaleBoth, -1, kVerticalFrame, ColorManager->Number2Pixel(17),true,false),
				       new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));
  DGScopeVerticalScale_DVS->SetRange(0,1);
  DGScopeVerticalScale_DVS->SetPosition(0,1);

  // ROOT embdedded canvas for display of waveforms and spectra
  DGScopeDisplayAndSlider_HF->AddFrame(DGScope_EC = new TRootEmbeddedCanvas("DGScope_EC", DGScopeDisplayAndSlider_HF, 650, 400),
				       new TGLayoutHints(kLHintsCenterX, 5,5,0,0));
  DGScope_EC->GetCanvas()->SetFillColor(0);
  DGScope_EC->GetCanvas()->SetFrameFillColor(19);
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

  TGHorizontalFrame *DGScopeDisplayButtons_HF = new TGHorizontalFrame(DGScopeDisplay_GF);
  DGScopeDisplay_GF->AddFrame(DGScopeDisplayButtons_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  // ROOT text button for starting/stopping data acquisition by the digitizer
  DGScopeDisplayButtons_HF->AddFrame(DGScopeStartStop_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Stopped", DGScopeStartStop_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  DGScopeStartStop_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  DGScopeStartStop_TB->Resize(300,30);
  DGScopeStartStop_TB->ChangeOptions(DGScopeStartStop_TB->GetOptions() | kFixedSize);
  DGScopeStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  DGScopeStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(1));


  // ROOT text button for manually triggering of DGScope acquisition
  DGScopeDisplayButtons_HF->AddFrame(DGScopeTrigger_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Manual trigger", DGScopeTrigger_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  DGScopeTrigger_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  DGScopeTrigger_TB->Resize(175,30);
  DGScopeTrigger_TB->ChangeOptions(DGScopeTrigger_TB->GetOptions() | kFixedSize);
  
  DGScopeDisplayButtons_HF->AddFrame(DGScopeUpdatePlot_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Update plot", DGScopeUpdatePlot_TB_ID),
				     new TGLayoutHints(kLHintsCenterX, 5,5,0,0));
  DGScopeUpdatePlot_TB->Resize(175,30);
  DGScopeUpdatePlot_TB->ChangeOptions(DGScopeUpdatePlot_TB->GetOptions() | kFixedSize);
  DGScopeUpdatePlot_TB->Connect("Clicked()","ADAQAcquisition", this, "HandleScopeButtons()");

  TGHorizontalFrame *DGScopeDisplayControls_HF = new TGHorizontalFrame(DGScopeDisplay_GF);
  DGScopeDisplay_GF->AddFrame(DGScopeDisplayControls_HF,
			      new TGLayoutHints(kLHintsCenterX,5,5,0,0));

  
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

  TGCompositeFrame *DGScopeSettingsTab = DGScopeControlTabs->AddTab("Acquisition control");
  TGCompositeFrame *DGScopeSettingsFrame = new TGCompositeFrame(DGScopeSettingsTab,0,0,kHorizontalFrame);
  DGScopeSettingsTab->AddFrame(DGScopeSettingsFrame);

  TGCompositeFrame *DGScopeSpectrumTab = DGScopeControlTabs->AddTab("Spectrum creation");
  TGCompositeFrame *DGScopeSpectrumFrame = new TGCompositeFrame(DGScopeSpectrumTab,0,0,kHorizontalFrame);
  DGScopeSpectrumTab->AddFrame(DGScopeSpectrumFrame);

  TGCompositeFrame *DGScopeDisplaySettingsTab = DGScopeControlTabs->AddTab("Graphic settings");
  TGCompositeFrame *DGScopeDisplaySettingsFrame = new TGCompositeFrame(DGScopeDisplaySettingsTab,0,0,kHorizontalFrame);
  DGScopeDisplaySettingsTab->AddFrame(DGScopeDisplaySettingsFrame);
  
  TGCompositeFrame *DGScopeDataStorageTab = DGScopeControlTabs->AddTab("Peristent storage");
  TGCompositeFrame *DGScopeDataStorageFrame = new TGCompositeFrame(DGScopeDataStorageTab,0,0,kHorizontalFrame);
  DGScopeDataStorageTab->AddFrame(DGScopeDataStorageFrame);

  TGCompositeFrame *DGScopeMiscTab = DGScopeControlTabs->AddTab("Miscellaneous");
  TGCompositeFrame *DGScopeMiscFrame = new TGCompositeFrame(DGScopeMiscTab,0,0,kHorizontalFrame);
  DGScopeMiscTab->AddFrame(DGScopeMiscFrame);


  ////////////////////
  // Scope settings //
  ////////////////////

  TGVerticalFrame *DGScopeModeAndTrigger_VF = new TGVerticalFrame(DGScopeSettingsFrame);
  DGScopeSettingsFrame->AddFrame(DGScopeModeAndTrigger_VF, new TGLayoutHints(kLHintsNormal,0,0,0,0));

  ////////////////
  // Mode controls 

  // ROOT radio buttons to specify operational mode of DGScope:
  // "Waveform" == oscilloscope, "Spectrum" == MCA, "Blank" == display
  // no graphics for high data throughput
  TGButtonGroup *DGScopeMode_BG = new TGButtonGroup(DGScopeModeAndTrigger_VF, "Scope Display Mode", kVerticalFrame);
  DGScopeMode_BG->SetTitlePos(TGButtonGroup::kCenter);
  DGScopeModeAndTrigger_VF->AddFrame(DGScopeMode_BG, new TGLayoutHints(kLHintsExpandX,5,5,5,5));
  
  DGScopeWaveform_RB = new TGRadioButton(DGScopeMode_BG, "Digitized waveform", DGScopeWaveform_RB_ID);
  DGScopeWaveform_RB->SetState(kButtonDown);

  DGScopeSpectrum_RB = new TGRadioButton(DGScopeMode_BG, "Pulse spectrum", DGScopeSpectrum_RB_ID);

  DGScopeHighRate_RB = new TGRadioButton(DGScopeMode_BG, "High-rate (updateable)", DGScopeHighRate_RB_ID);

  DGScopeUltraHighRate_RB = new TGRadioButton(DGScopeMode_BG, "Ultra-rate (non-updateable)", DGScopeUltraHighRate_RB_ID);
  
  DGScopeMode_BG->Show();

  ///////////////////
  // Trigger controls
  
  TGGroupFrame *DGScopeTriggerControls_GF = new TGGroupFrame(DGScopeModeAndTrigger_VF, "Trigger Control", kVerticalFrame);
  DGScopeTriggerControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeModeAndTrigger_VF->AddFrame(DGScopeTriggerControls_GF, new TGLayoutHints(kLHintsNormal,5,5,5,0));
  
  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerCoincidenceEnable_CB = new TGCheckButton(DGScopeTriggerControls_GF, "Coincidence triggering",DGScopeTriggerCoincidenceEnable_CB_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,0));
  
  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerCoincidenceLevel_CBL = new ADAQComboBoxWithLabel(DGScopeTriggerControls_GF, "Coincidence", -1),
				      new TGLayoutHints(kLHintsNormal,5,5,0,5));
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

  
  ///////////////////////
  // Acquisition controls

  TGGroupFrame *DGScopeAcquisitionControls_GF = new TGGroupFrame(DGScopeSettingsFrame, "Acquisition", kVerticalFrame);
  DGScopeAcquisitionControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSettingsFrame->AddFrame(DGScopeAcquisitionControls_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ number entry specifying number of samples
  DGScopeAcquisitionControls_GF->AddFrame(DGScopeRecordLength_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Record length (#)", DGScopeRecordLength_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeRecordLength_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeRecordLength_NEL->GetEntry()->SetNumber(2000);

  // ADAQ number entry specifying the percentage of the acquisition
  // window that is behind (or after) the triggern (all channels)
  DGScopeAcquisitionControls_GF->AddFrame(DGScopePostTriggerSize_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Post trigger (%)", DGScopePostTriggerSize_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
  DGScopePostTriggerSize_NEL->GetEntry()->SetLimitValues(0,100);
  DGScopePostTriggerSize_NEL->GetEntry()->SetNumber(50);
  
  DGScopeAcquisitionControls_GF->AddFrame(DGScopeAcquisitionTime_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Acquisition time [s]", -1),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeAcquisitionTime_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeAcquisitionTime_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeAcquisitionTime_NEL->GetEntry()->SetNumber(10);

  DGScopeAcquisitionControls_GF->AddFrame(DGScopeAcquisitionTimer_NEFL = new ADAQNumberEntryFieldWithLabel(DGScopeAcquisitionControls_GF, "Countdown", -1),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeAcquisitionTimer_NEFL->GetEntry()->SetFormat(TGNumberFormat::kNESMinSec);
  DGScopeAcquisitionTimer_NEFL->GetEntry()->SetNumber(10);
  DGScopeAcquisitionTimer_NEFL->GetEntry()->SetState(false);

  TGHorizontalFrame *DGScopeTimerButtons_HF = new TGHorizontalFrame(DGScopeAcquisitionControls_GF);
  DGScopeAcquisitionControls_GF->AddFrame(DGScopeTimerButtons_HF);
  
  DGScopeTimerButtons_HF->AddFrame(DGScopeAcquisitionTimerStart_TB = new TGTextButton(DGScopeTimerButtons_HF, "Start timer", DGScopeAcquisitionTimerStart_TB_ID),
				   new TGLayoutHints(kLHintsNormal, 5,5,0,0));
  DGScopeAcquisitionTimerStart_TB->Resize(100, 30);
  DGScopeAcquisitionTimerStart_TB->ChangeOptions(DGScopeAcquisitionTimerStart_TB->GetOptions() | kFixedSize);
  DGScopeAcquisitionTimerStart_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  
  DGScopeTimerButtons_HF->AddFrame(DGScopeAcquisitionTimerAbort_TB = new TGTextButton(DGScopeTimerButtons_HF, "Abort timer", DGScopeAcquisitionTimerAbort_TB_ID),
				   new TGLayoutHints(kLHintsNormal, 5,5,0,0));
  DGScopeAcquisitionTimerAbort_TB->Resize(100, 30);
  DGScopeAcquisitionTimerAbort_TB->ChangeOptions(DGScopeAcquisitionTimerAbort_TB->GetOptions() | kFixedSize);
  DGScopeAcquisitionTimerAbort_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  


  // V1720 readout controls
  TGGroupFrame *DGScopeReadoutControls_GF = new TGGroupFrame(DGScopeSettingsFrame, "Readout", kVerticalFrame);
  DGScopeReadoutControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSettingsFrame->AddFrame(DGScopeReadoutControls_GF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeReadoutControls_GF->AddFrame(DGScopeMaxEventsBeforeTransfer_NEL = new ADAQNumberEntryWithLabel(DGScopeReadoutControls_GF, "ME transfer events", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeMaxEventsBeforeTransfer_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeMaxEventsBeforeTransfer_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeMaxEventsBeforeTransfer_NEL->GetEntry()->SetNumber(5);

  DGScopeReadoutControls_GF->AddFrame(DGScopeCheckBufferStatus_TB = new TGTextButton(DGScopeReadoutControls_GF, "Check V1720 Buffer", DGScopeCheckBufferStatus_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeCheckBufferStatus_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  DGScopeCheckBufferStatus_TB->Resize(150,30);
  DGScopeCheckBufferStatus_TB->ChangeOptions(DGScopeCheckBufferStatus_TB->GetOptions() | kFixedSize);
  
  DGScopeReadoutControls_GF->AddFrame(DGScopeBufferStatus_TE = new TGTextEntry(DGScopeReadoutControls_GF, "<Click above check!>",-1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeBufferStatus_TE->SetAlignment(kTextCenterX);
  DGScopeBufferStatus_TE->Resize(200,30);
  DGScopeBufferStatus_TE->ChangeOptions(DGScopeBufferStatus_TE->GetOptions() | kFixedSize);


  DGScopeReadoutControls_GF->AddFrame(DGScopeUseDataReduction_CB = new TGCheckButton(DGScopeReadoutControls_GF, "Use data reduction", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));

  DGScopeReadoutControls_GF->AddFrame(DGScopeDataReductionFactor_NEL = new ADAQNumberEntryWithLabel(DGScopeReadoutControls_GF, "Data reduction factor", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  DGScopeDataReductionFactor_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeDataReductionFactor_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeDataReductionFactor_NEL->GetEntry()->SetNumber(1);

  DGScopeReadoutControls_GF->AddFrame(DGScopeZSMode_CBL = new ADAQComboBoxWithLabel(DGScopeReadoutControls_GF, "ZS Mode", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  DGScopeZSMode_CBL->GetComboBox()->Resize(130,20);
  DGScopeZSMode_CBL->GetComboBox()->AddEntry("Disabled", 0);
  DGScopeZSMode_CBL->GetComboBox()->AddEntry("Z-length encoding", 2);
  DGScopeZSMode_CBL->GetComboBox()->AddEntry("Z-suppression", 3);
  DGScopeZSMode_CBL->GetComboBox()->Select(0);
  


  ///////////////////////
  // Spectrum settings //
  ///////////////////////

  ////////////
  // Histogram  

  TGGroupFrame *DGScopeSpectrumHistogram_GF = new TGGroupFrame(DGScopeSpectrumFrame, "Histogram", kVerticalFrame);
  DGScopeSpectrumHistogram_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumFrame->AddFrame(DGScopeSpectrumHistogram_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));

  // ADAQ combo box for selecting the channel for display spectrum
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumChannel_CBL = new ADAQComboBoxWithLabel(DGScopeSpectrumHistogram_GF, "", DGScopeSpectrumChannel_CBL_ID),
					new TGLayoutHints(kLHintsNormal,0,0,5,5));
  for(uint32_t ch=0; ch<8; ch++)
    DGScopeSpectrumChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  DGScopeSpectrumChannel_CBL->GetComboBox()->Select(0);
  DGScopeSpectrumChannel_CBL->GetComboBox()->Connect("Selected(int,int)", "ADAQAcquisition", this, "HandleComboBoxes(int,int)");
  
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
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetNumber(0.);

  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumMaxBin_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumHistogram_GF, "Maximum bin", DGScopeSpectrumMaxBin_NEL_ID),
					new TGLayoutHints(kLHintsLeft,0,0,0,5));
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetNumber(30000.);

  TGHorizontalFrame *DGScopeSpectrumAxis_HF = new TGHorizontalFrame(DGScopeSpectrumHistogram_GF);
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumAxis_HF, new TGLayoutHints(kLHintsNormal,0,0,0,0));

  // ROOT check button that allows multiple runs ("run" == acquisition
  // on and then off) to sum into the same histogram
  DGScopeSpectrumHistogram_GF->AddFrame(DGScopeSpectrumAggregateRuns_CB = new TGCheckButton(DGScopeSpectrumHistogram_GF, "Aggregate runs", DGScopeSpectrumAggregateRuns_CB_ID),
					new TGLayoutHints(kLHintsLeft,0,0,5,5));


  /////////////////
  // Pulse analysis

  TGGroupFrame *DGScopeSpectrumAnalysis_GF = new TGGroupFrame(DGScopeSpectrumFrame,"Analysis",kVerticalFrame);
  DGScopeSpectrumAnalysis_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeSpectrumFrame->AddFrame(DGScopeSpectrumAnalysis_GF);

  TGHButtonGroup *DGScopeSpectrumAnalysis_BG = new TGHButtonGroup(DGScopeSpectrumAnalysis_GF,"Analysis");
  DGScopeSpectrumAnalysis_BG->SetBorderDrawn(false);
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysis_BG, new TGLayoutHints(kLHintsNormal,-13,0,0,0));
  
  DGScopeSpectrumAnalysisHeight_RB = new TGRadioButton(DGScopeSpectrumAnalysis_BG, "PHS  ", DGScopeSpectrumAnalysisHeight_RB_ID);
  DGScopeSpectrumAnalysisHeight_RB->Connect("Clicked()", "ADAQAcquisition", this, "HandleRadioButtons()");

  
  DGScopeSpectrumAnalysisArea_RB = new TGRadioButton(DGScopeSpectrumAnalysis_BG, "PAS", DGScopeSpectrumAnalysisArea_RB_ID);
  DGScopeSpectrumAnalysisArea_RB->Connect("Clicked()", "ADAQAcquisition", this, "HandleRadioButtons()");
  DGScopeSpectrumAnalysisArea_RB->SetState(kButtonDown);
  
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLLD_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumAnalysis_GF, "LLD (ADC/energy)", -1),
				       new TGLayoutHints(kLHintsNormal,0,0,-2,0));
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(0);

  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisULD_NEL = new ADAQNumberEntryWithLabel(DGScopeSpectrumAnalysis_GF, "ULD (ADC/energy)", -1),
				       new TGLayoutHints(kLHintsNormal,0,0,0,0));
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(100000);

  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLDTrigger_CB = new TGCheckButton(DGScopeSpectrumAnalysis_GF,"LD trigger to file", -1),
				       new TGLayoutHints(kLHintsNormal,0,0,0,0));
  
  DGScopeSpectrumAnalysis_GF->AddFrame(DGScopeSpectrumAnalysisLDTriggerChannel_CBL = new ADAQComboBoxWithLabel(DGScopeSpectrumAnalysis_GF, "LD Channel", -1),
				       new TGLayoutHints(kLHintsNormal,0,0,0,5));
  
  for(uint32_t ch=0; ch<8; ch++)
    DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->Select(0);

  
  //////////////
  // Calibration

  TGGroupFrame *SpectrumCalibration_GF = new TGGroupFrame(DGScopeSpectrumFrame, "Energy calibration", kVerticalFrame);
  DGScopeSpectrumFrame->AddFrame(SpectrumCalibration_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));
  
  TGHorizontalFrame *SpectrumCalibration_HF0 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  // Energy calibration 
  SpectrumCalibration_HF0->AddFrame(DGScopeSpectrumCalibration_CB = new TGCheckButton(SpectrumCalibration_HF0, "Make it so", DGScopeSpectrumCalibration_CB_ID),
				    new TGLayoutHints(kLHintsLeft, 0,0,5,0));
  DGScopeSpectrumCalibration_CB->Connect("Clicked()", "ADAQAcquisition", this, "HandleCheckButtons()");
  DGScopeSpectrumCalibration_CB->SetState(kButtonUp);

  
  SpectrumCalibration_HF0->AddFrame(DGScopeSpectrumUseCalibrationSlider_CB = new TGCheckButton(SpectrumCalibration_HF0, "Use slider", DGScopeSpectrumUseCalibrationSlider_CB_ID),
				    new TGLayoutHints(kLHintsLeft,25,5,5,0));
  DGScopeSpectrumUseCalibrationSlider_CB->Connect("Clicked()","ADAQAcquisition",this,"HandleCheckButtons()");
  DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonDown);
  DGScopeSpectrumUseCalibrationSlider_CB->SetEnabled(false);

  /*
  SpectrumCalibration_HF1->AddFrame(SpectrumCalibrationStandard_RB = new TGRadioButton(SpectrumCalibration_HF1, "Standard", SpectrumCalibrationStandard_RB_ID),
					   new TGLayoutHints(kLHintsNormal, 10,5,5,5));
  SpectrumCalibrationStandard_RB->SetState(kButtonDown);
  SpectrumCalibrationStandard_RB->SetState(kButtonDisabled);
  SpectrumCalibrationStandard_RB->Connect("Clicked()", "ADAQAcquisition", this, "HandleRadioButtons()");
  
  SpectrumCalibration_HF1->AddFrame(SpectrumCalibrationEdgeFinder_RB = new TGRadioButton(SpectrumCalibration_HF1, "Edge finder", SpectrumCalibrationEdgeFinder_RB_ID),
				       new TGLayoutHints(kLHintsNormal, 30,5,5,5));
  SpectrumCalibrationEdgeFinder_RB->SetState(kButtonDisabled);
  SpectrumCalibrationEdgeFinder_RB->Connect("Clicked()", "ADAQAcquisition", this, "HandleRadioButtons()");


  TGVerticalFrame *SpectrumCalibration_VF = new TGVerticalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_VF);
  */

  TGHorizontalFrame *SpectrumCalibration_HF1 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF1, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  SpectrumCalibration_HF1->AddFrame(DGScopeSpectrumCalibrationPoint_CBL = new ADAQComboBoxWithLabel(SpectrumCalibration_HF1, "", DGScopeSpectrumCalibrationPoint_CBL_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,10,3));
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Resize(150,20);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0",0);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(false);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Connect("Selected(int,int)", "ADAQAcquisition", this, "HandleComboBoxes(int,int)");
  DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonDisabled);
  
  SpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationEnergy_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Energy (keV or MeV)", DGScopeSpectrumCalibrationEnergy_NEL_ID),
				   new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->Connect("ValueSet(long)", "ADAQAcquisition", this, "HandleScopeNumberEntries()");

  SpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationPulseUnit_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Pulse unit (ADC)", DGScopeSpectrumCalibrationPulseUnit_NEL_ID),
				       new TGLayoutHints(kLHintsLeft,0,0,0,5));
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->Connect("ValueSet(long)", "ADAQAcquisition", this, "HandleScopeNumberEntries()");

  TGHorizontalFrame *SpectrumCalibration_HF2 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF2);
  
  // Set point text button
  SpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationSetPoint_TB = new TGTextButton(SpectrumCalibration_HF2, "Set Pt.", DGScopeSpectrumCalibrationSetPoint_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationSetPoint_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationSetPoint_TB->Resize(100,25);
  DGScopeSpectrumCalibrationSetPoint_TB->ChangeOptions(DGScopeSpectrumCalibrationSetPoint_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);

  // Calibrate text button
  SpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationCalibrate_TB = new TGTextButton(SpectrumCalibration_HF2, "Calibrate", DGScopeSpectrumCalibrationCalibrate_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationCalibrate_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationCalibrate_TB->Resize(100,25);
  DGScopeSpectrumCalibrationCalibrate_TB->ChangeOptions(DGScopeSpectrumCalibrationCalibrate_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);
  
  TGHorizontalFrame *SpectrumCalibration_HF3 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF3);
  
  // Plot text button
  SpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationPlot_TB = new TGTextButton(SpectrumCalibration_HF3, "Plot", DGScopeSpectrumCalibrationPlot_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationPlot_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationPlot_TB->Resize(100,25);
  DGScopeSpectrumCalibrationPlot_TB->ChangeOptions(DGScopeSpectrumCalibrationPlot_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonDisabled);

  // Reset text button
  SpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationReset_TB = new TGTextButton(SpectrumCalibration_HF3, "Reset", DGScopeSpectrumCalibrationReset_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationReset_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationReset_TB->Resize(100,25);
  DGScopeSpectrumCalibrationReset_TB->ChangeOptions(DGScopeSpectrumCalibrationReset_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationReset_TB->SetState(kButtonDisabled);


  TGHorizontalFrame *SpectrumCalibration_HF4 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF4);

  // Load from file text button
  SpectrumCalibration_HF4->AddFrame(DGScopeSpectrumCalibrationLoad_TB = new TGTextButton(SpectrumCalibration_HF4, "Load", DGScopeSpectrumCalibrationLoad_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationLoad_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationLoad_TB->Resize(100,25);
  DGScopeSpectrumCalibrationLoad_TB->ChangeOptions(DGScopeSpectrumCalibrationLoad_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationLoad_TB->SetState(kButtonDisabled);

  // Write to file text button
  SpectrumCalibration_HF4->AddFrame(DGScopeSpectrumCalibrationWrite_TB = new TGTextButton(SpectrumCalibration_HF4, "Write", DGScopeSpectrumCalibrationWrite_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationWrite_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeSpectrumCalibrationWrite_TB->Resize(100,25);
  DGScopeSpectrumCalibrationWrite_TB->ChangeOptions(DGScopeSpectrumCalibrationLoad_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationWrite_TB->SetState(kButtonDisabled);

  //////////////////////
  // Display settings //
  //////////////////////

  ////////////////////////////
  // Title names and positions 

  TGGroupFrame *DGScopeDisplayTitle_GF = new TGGroupFrame(DGScopeDisplaySettingsFrame, "Titles", kVerticalFrame);
  DGScopeDisplayTitle_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDisplaySettingsFrame->AddFrame(DGScopeDisplayTitle_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ text entries and number entries for specifying the DGScope title, axes title, and axes position

  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayXTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "X-axis title", -1),
				   new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeDisplayXTitle_TEL->GetEntry()->SetText("");

  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayXTitleOffset_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayTitle_GF, "X-axis title offset", -1),
				   new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeDisplayXTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayXTitleOffset_NEL->GetEntry()->SetNumber(1.2);

  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayYTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "Y-axis label", -1),
				   new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGScopeDisplayYTitle_TEL->GetEntry()->SetText("");

  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayYTitleOffset_NEL = new ADAQNumberEntryWithLabel(DGScopeDisplayTitle_GF, "Y-axis offset", -1),
				   new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeDisplayYTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeDisplayYTitleOffset_NEL->GetEntry()->SetNumber(1.5);
  
  DGScopeDisplayTitle_GF->AddFrame(DGScopeDisplayTitle_TEL = new ADAQTextEntryWithLabel(DGScopeDisplayTitle_GF, "Graph Title", -1),
				   new TGLayoutHints(kLHintsNormal,5,5,5,5));

  //////////////////////////
  // Graphical attributes //
  //////////////////////////
  
  TGGroupFrame *DGScopeDisplaySettings_GF = new TGGroupFrame(DGScopeDisplaySettingsFrame, "Display", kVerticalFrame);
  DGScopeDisplaySettings_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDisplaySettingsFrame->AddFrame(DGScopeDisplaySettings_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ROOT check button to enable/disable plotting of the legend
  DGScopeDisplaySettings_GF->AddFrame(DGScopeDisplayDrawLegend_CB = new TGCheckButton(DGScopeDisplaySettings_GF, "Draw legend", -1),
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


  //////////////////
  // Data storage //
  //////////////////

  TGGroupFrame *DGScopeDataStorage_GF = new TGGroupFrame(DGScopeDataStorageFrame, "Waveform storage", kVerticalFrame);
  DGScopeDataStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDataStorageFrame->AddFrame(DGScopeDataStorage_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ text entry for setting the ROOT file name
  /*
  DGScopeDataStorage_GF->AddFrame(DGScopeDataFileName_TEL = new ADAQTextEntryWithLabel(DGScopeDataStorage_GF, "Filename",-1),
				  new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDataFileName_TEL->GetEntry()->SetText("ADAQMeasurement");
  DGScopeDataFileName_TEL->GetEntry()->SetState(false);

  // ADAQ text entry that will add a comment on the saved data to the ROOT file, making it possible to
  // record various parameters, settings, etc on the measurement for later retrieval
  DGScopeDataStorage_GF->AddFrame(DGScopeDataComment_TEL = new ADAQTextEntryWithLabel(DGScopeDataStorage_GF, "Comment", -1),
				  new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeDataComment_TEL->GetEntry()->SetState(false);
  */
  DGScopeDataStorage_GF->AddFrame(DGScopeDataFileName_TB = new TGTextButton(DGScopeDataStorage_GF, "Data file name", DGScopeDataFileName_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,5,0));
  DGScopeDataFileName_TB->Connect("Clicked()","ADAQAcquisition", this, "HandleScopeButtons()");
  DGScopeDataFileName_TB->Resize(175, 30);
  DGScopeDataFileName_TB->ChangeOptions(DGScopeDataFileName_TB->GetOptions() | kFixedSize);

  DGScopeDataStorage_GF->AddFrame(DGScopeDataFileName_TEL = new ADAQTextEntryWithLabel(DGScopeDataStorage_GF, "", -1),
				      new TGLayoutHints(kLHintsNormal,10,5,5,0));
  DGScopeDataFileName_TEL->GetEntry()->Resize(175, 25);
  DGScopeDataFileName_TEL->GetEntry()->ChangeOptions(DGScopeDataFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  DGScopeDataFileName_TEL->GetEntry()->SetState(false);
  DGScopeDataFileName_TEL->GetEntry()->SetText("DefaultData.adaq");

  // ROOT text button to create a root file using the name in the text entry field above
  DGScopeDataStorage_GF->AddFrame(DGScopeDataStorageCreateFile_TB = new TGTextButton(DGScopeDataStorage_GF,"Create ADAQ file", DGScopeDataStorageCreateFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,8,5));
  DGScopeDataStorageCreateFile_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  DGScopeDataStorageCreateFile_TB->Resize(175,30);
  DGScopeDataStorageCreateFile_TB->ChangeOptions(DGScopeDataStorageCreateFile_TB->GetOptions() | kFixedSize);
  DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);

  // ROOT text button to write all data to the ROOT file and close it. This button MUST be clicked to 
  // successfully write&close the ROOT file otherwise the ROOT file will have errors.
  DGScopeDataStorage_GF->AddFrame(DGScopeDataStorageCloseFile_TB = new TGTextButton(DGScopeDataStorage_GF,"Close ADAQ file", DGScopeDataStorageCloseFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,0,5));
  DGScopeDataStorageCloseFile_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  DGScopeDataStorageCloseFile_TB->Resize(175,30);
  DGScopeDataStorageCloseFile_TB->ChangeOptions(DGScopeDataStorageCloseFile_TB->GetOptions() | kFixedSize);
  DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
  
  // ROOT check button to enable/disable saving data to ROOT file. Note that the data is saved to
  // the ROOT file only while the button is checked. The 
  DGScopeDataStorage_GF->AddFrame(DGScopeDataStorageEnable_CB = new TGCheckButton(DGScopeDataStorage_GF,"Data stored while checked", -1),
				  new TGLayoutHints(kLHintsNormal,10,5,5,5));
  DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);
  
  DGScopeDisplayAndControls_VF->AddFrame(DGScopeDisplay_GF, new TGLayoutHints(kLHintsCenterX,5,5,5,5));
  DGScopeDisplayAndControls_VF->AddFrame(DGScopeControlTabs_HF, new TGLayoutHints(kLHintsCenterX,5,5,5,5));

  ScopeFrame->AddFrame(DGScopeDisplayAndControls_VF, new TGLayoutHints(kLHintsNormal,5,5,5,5));


  // Widgets for saving the spectrum data to file

  TGGroupFrame *DGScopeSpectrumStorage_GF = new TGGroupFrame(DGScopeDataStorageFrame, "Spectrum output", kVerticalFrame);
  DGScopeSpectrumStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDataStorageFrame->AddFrame(DGScopeSpectrumStorage_GF, new TGLayoutHints(kLHintsNormal,0,5,5,5));

  DGScopeSpectrumStorage_GF->AddFrame(DGScopeSpectrumFileName_TB = new TGTextButton(DGScopeSpectrumStorage_GF, "Spectrum file name", DGScopeSpectrumFileName_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumFileName_TB->Resize(175, 30);
  DGScopeSpectrumFileName_TB->ChangeOptions(DGScopeSpectrumFileName_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumFileName_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  
  DGScopeSpectrumStorage_GF->AddFrame(DGScopeSpectrumFileName_TEL = new ADAQTextEntryWithLabel(DGScopeSpectrumStorage_GF, "", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumFileName_TEL->GetEntry()->Resize(175, 25);
  DGScopeSpectrumFileName_TEL->GetEntry()->ChangeOptions(DGScopeSpectrumFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  DGScopeSpectrumFileName_TEL->GetEntry()->SetState(false);
  DGScopeSpectrumFileName_TEL->GetEntry()->SetText("DefaultSpectrum.dat");

  DGScopeSpectrumStorage_GF->AddFrame(DGScopeSaveSpectrumWithTimeExtension_CB = new TGCheckButton(DGScopeSpectrumStorage_GF, "Add time to file name", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeSpectrumStorage_GF->AddFrame(DGScopeSaveSpectrum_TB = new TGTextButton(DGScopeSpectrumStorage_GF, "Save spectrum data", DGScopeSaveSpectrum_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  DGScopeSaveSpectrum_TB->Resize(175, 30);
  DGScopeSaveSpectrum_TB->ChangeOptions(DGScopeSaveSpectrum_TB->GetOptions() | kFixedSize);
  DGScopeSaveSpectrum_TB->Connect("Clicked()","ADAQAcquisition", this, "HandleScopeButtons()");


  // Widgets for saving the canvas graphics to file
  
  TGGroupFrame *DGScopeCanvasStorage_GF = new TGGroupFrame(DGScopeDataStorageFrame, "Canvas output", kVerticalFrame);
  DGScopeCanvasStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDataStorageFrame->AddFrame(DGScopeCanvasStorage_GF, new TGLayoutHints(kLHintsNormal,0,0,5,5));

  DGScopeCanvasStorage_GF->AddFrame(DGScopeCanvasFileName_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Canvas file name", DGScopeCanvasFileName_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeCanvasFileName_TB->Resize(175, 30);
  DGScopeCanvasFileName_TB->ChangeOptions(DGScopeCanvasFileName_TB->GetOptions() | kFixedSize);
  DGScopeCanvasFileName_TB->Connect("Clicked()", "ADAQAcquisition", this, "HandleScopeButtons()");
  
  DGScopeCanvasStorage_GF->AddFrame(DGScopeCanvasFileName_TEL = new ADAQTextEntryWithLabel(DGScopeCanvasStorage_GF, "", -1),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeCanvasFileName_TEL->GetEntry()->Resize(175, 25);
  DGScopeCanvasFileName_TEL->GetEntry()->ChangeOptions(DGScopeCanvasFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  DGScopeCanvasFileName_TEL->GetEntry()->SetState(false);
  DGScopeCanvasFileName_TEL->GetEntry()->SetText("DefaultGraphics.eps");

  DGScopeCanvasStorage_GF->AddFrame(DGScopeSaveCanvasWithTimeExtension_CB = new TGCheckButton(DGScopeCanvasStorage_GF, "Add time to file name", -1),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeCanvasStorage_GF->AddFrame(DGScopeSaveCanvas_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Save canvas graphics", DGScopeSaveCanvas_TB_ID),
				    new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGScopeSaveCanvas_TB->Resize(175, 30);
  DGScopeSaveCanvas_TB->ChangeOptions(DGScopeSaveCanvas_TB->GetOptions() | kFixedSize);
  DGScopeSaveCanvas_TB->Connect("Clicked()","ADAQAcquisition",this,"HandleScopeButtons()");
  

  ///////////////////
  // Miscellaneous //
  ///////////////////

  TGGroupFrame *DGScopeMisc_GF = new TGGroupFrame(DGScopeMiscFrame, "Extra options", kVerticalFrame);
  DGScopeMisc_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeMiscFrame->AddFrame(DGScopeMisc_GF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));

  DGScopeMisc_GF->AddFrame(DGScopeSpectrumRefreshRate_NEL = new ADAQNumberEntryWithLabel(DGScopeMisc_GF, "Spectrum refresh rate", -1),
			   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetNumber(100);
  
  if(BuildInDebugMode){
    TGGroupFrame *DebugMode_GF = new TGGroupFrame(DGScopeMiscFrame, "Debugging options", kVerticalFrame);
    DebugMode_GF->SetTitlePos(TGGroupFrame::kCenter);
    DGScopeMiscFrame->AddFrame(DebugMode_GF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));
    
    DebugMode_GF->AddFrame(DebugModeEnable_CB = new TGCheckButton(DebugMode_GF, "Enable debugging mode", -1),
			   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
			       
    DebugMode_GF->AddFrame(DebugModeWaveformGenerationPause_NEL = new ADAQNumberEntryWithLabel(DebugMode_GF, "Waveform generation pause [us]", -1),
			   new TGLayoutHints(kLHintsNormal, 5,5,5,5));
    DebugModeWaveformGenerationPause_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DebugModeWaveformGenerationPause_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    DebugModeWaveformGenerationPause_NEL->GetEntry()->SetNumber(10000);
  }
}


// Handles all actions by buttons on the VME Connection frame
void ADAQAcquisition::HandleConnectionButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();

  enum{V1718, V1720, V6534};

  // Temporarily redirect the std::cout messages to a local buffer
  streambuf* StandardBuffer = cout.rdbuf();
  ostringstream NewBuffer;
  cout.rdbuf( NewBuffer.rdbuf() );

  switch(ActiveButtonID){

    // Connect ADAQAcquisition with VME boards
  case V1718Connect_TB_ID:
    
    // If no connection is presently established...
    if(!VMEConnectionEstablished){

      int V1720LinkOpen = -42;
      if(V1720Enable){
	V1720BoardAddress = BoardAddress_NEF[V1720]->GetHexNumber();
	V1720LinkOpen = DGManager->OpenLink(V1720BoardAddress);
	DGManager->Initialize();
      }
      
      int V1718LinkOpen = -42;
      if(V1718Enable and V1720LinkOpen == 0){
	V1718LinkOpen = BRManager->OpenLink(DGManager->GetBoardHandle(), true);
      }

      int V6534LinkOpen = -42;
      if(V6534Enable){
	V6534BoardAddress = BoardAddress_NEF[V6534]->GetHexNumber();
	V6534LinkOpen = HVManager->OpenLink(V6534BoardAddress);

	if(V6534LinkOpen == 0)
	  HVManager->SetToSafeState();
      }

      if(V6534LinkOpen==0 or V1720LinkOpen==0){
	V1718Connect_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
	V1718Connect_TB->SetText("Connected: click to disconnect");
	VMEConnectionEstablished = true;

	// Convert the new "std::cout" buffer into a TGText
	string InputString = NewBuffer.str();
	TGText *InputText = new TGText;
	InputText->LoadBuffer(InputString.c_str());

	// Update the connection TGTextView with the status messages
	ConnectionOutput_TV->AddText(InputText);
	ConnectionOutput_TV->ShowBottom();
	ConnectionOutput_TV->Update();
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

      // Convert the new "std::cout" buffer into a TGText
      string InputString = NewBuffer.str();
      TGText *InputText = new TGText;
      InputText->LoadBuffer(InputString.c_str());
      
      // Update the connection TGTextView with the status messages
      ConnectionOutput_TV->AddText(InputText);
      ConnectionOutput_TV->ShowBottom();
      ConnectionOutput_TV->Update();
    }
    break;

    // Set the V6534Enable boolean that controls whether or not the
    // V6534 high voltage board should be presently used
  case V6534BoardEnable_TB_ID:
    if(BoardEnable_TB[V6534]->GetString() == "Board enabled"){
      BoardEnable_TB[V6534]->SetText("Board disabled");
      BoardEnable_TB[V6534]->SetBackgroundColor(ColorManager->Number2Pixel(2));
      V6534Enable = false;
    }
    else if(BoardEnable_TB[V6534]->GetString() == "Board disabled"){
      BoardEnable_TB[V6534]->SetText("Board enabled");
      BoardEnable_TB[V6534]->SetBackgroundColor(ColorManager->Number2Pixel(8));
      V6534Enable = true;
    }
    break;

    // Set the V1720Enable boolean that controls whether or not the
    // V1720 high voltage board should be presently used
  case V1720BoardEnable_TB_ID:
    if(BoardEnable_TB[V1720]->GetString() == "Board enabled"){
      BoardEnable_TB[V1720]->SetText("Board disabled");
      BoardEnable_TB[V1720]->SetBackgroundColor(ColorManager->Number2Pixel(2));
      V1720Enable = false;
    }
    else if(BoardEnable_TB[V1720]->GetString() == "Board disabled"){
      BoardEnable_TB[V1720]->SetText("Board enabled");
      BoardEnable_TB[V1720]->SetBackgroundColor(ColorManager->Number2Pixel(8));
      V1720Enable = true;
    }
    break;
  }

  // Redirect std::cout back to the normal terminal output
  cout.rdbuf(StandardBuffer);
}

void ADAQAcquisition::HandleRegisterButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();

  // 32-bit integers to hold register addresses and data; 16-bit
  // integer to hold data obtained from the V6534 board
  uint32_t addr32 = 0;
  uint32_t data32 = 0;
  uint16_t data16 = 0;

  // Create two enums for local parsing of the actions
  enum{READ,WRITE};

  // Use a case statement to two select the action and the board upon
  // which the register read/write will take place
  int Action = 0;
  int Board = 0;
  switch(ActiveButtonID){

  case V1718Read_ID:
    Action = READ;
    Board = V1718;
    break;

  case V1718Write_ID:
    Action = WRITE;
    Board = V1718;
    break;

  case V1720Read_ID:
    Action = READ;
    Board = V1720;
    break;

  case V1720Write_ID:
    Action = WRITE;
    Board = V1720;
    break;
    
  case V6534Read_ID:
    Action = READ;
    Board = V6534;
    break;

  case V6534Write_ID:
    Action = WRITE;
    Board = V6534;
    break;
  }

  ///////////////////////////////////////////////
  // Perform a register read of the desired board
  if(Action == READ){
    addr32 = ReadAddress_NEF[Board]->GetHexNumber();

    cout << V1718Enable << endl;

    if(Board == V1718 and V1718Enable){
      BRManager->GetRegisterValue(addr32, &data32);
      cout << data32 << endl;
    }
    else if(Board == V1720 and V1720Enable)
      DGManager->GetRegisterValue(addr32, &data32);
    else if(Board == V6534 and V6534Enable){
      HVManager->GetRegisterValue(addr32, &data16);
      data32 = data16;
    }

    // Update the hex widget with the register value
    ReadValueHex_NEF[Board]->SetHexNumber(data32);

    // Update the binary widget with the register value inserting
    // spaces every 4 characters for easier reading
    bitset<32> Value (data32);
    string ValueString = Value.to_string();
    int offset = 0;
    for(int i=0; i<32/4; i++){
      ValueString.insert(i*4+offset, " ");
      offset++;
    }
    ReadValueBinary_TE[Board]->SetText(ValueString.c_str());
  }

  ////////////////////////////////////////////////
  // Perform a register write of the desired board
  else if(Action == WRITE){
    addr32 = WriteAddress_NEF[Board]->GetHexNumber();
    data32 = WriteValue_NEF[Board]->GetHexNumber();

    if(Board == V1718)
      BRManager->SetRegisterValue(addr32, data32);
    if(Board == V1720 and V1720Enable) 
      DGManager->SetRegisterValue(addr32, data32);
    else if(Board == V6534 and V6534Enable)
      HVManager->SetRegisterValue(addr32, data32);
  }
}


void ADAQAcquisition::HandlePulserButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();

  if(!VMEConnectionEstablished)
    return;

  enum {PulserA, PulserB};
  int Pulser = -1;

  switch(ActiveButtonID){

  case V1718PulserA_TB_ID:
    Pulser = PulserA;
    break;

  case V1718PulserB_TB_ID:
    Pulser = PulserB;
    break;
  }

  // Readout the widgets into structs provided by ADAQBridge class
  PulserSettings PS;
  PS.PulserToSet = Pulser;
  PS.Period = V1718PulserPeriod_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.Width = V1718PulserWidth_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.TimeUnit = V1718PulserTimeUnit_CBL[Pulser]->GetComboBox()->GetSelected();
  PS.PulseNumber = V1718PulserPulses_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.StartSource = V1718PulserStartSource_CBL[Pulser]->GetComboBox()->GetSelected();
  PS.StopSource = V1718PulserStopSource_CBL[Pulser]->GetComboBox()->GetSelected();

  PulserOutputSettings POS;
  POS.OutputLine = V1718PulserOutputLine_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.OutputPolarity = V1718PulserOutputPolarity_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.LEDPolarity = V1718PulserLEDPolarity_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.Source = V1718PulserSource_CBL[Pulser]->GetComboBox()->GetSelected();

  // Update the pulser settings through the V1718 manager class
  BRManager->SetPulserSettings(&PS);
  BRManager->SetPulserOutputSettings(&POS);

  if(ActiveTextButton->GetString()=="Stopped"){
    // Update button color from red to green andn update text
    ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(8));
    ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
    ActiveTextButton->SetText("Pulsing");

    BRManager->StartPulser(Pulser);
  }
  else if(ActiveTextButton->GetString()=="Pulsing"){
    // Update button color from green to red and update text
    ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(2));
    ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
    ActiveTextButton->SetText("Stopped");

    BRManager->StopPulser(Pulser);
  }
}


// Perform actions that are activated by the text buttons on the HV frame
void ADAQAcquisition::HandleVoltageButtons()
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
      // ADAQAcquisition::RunHVMonitoring)
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
void ADAQAcquisition::HandleScopeButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveTextButton = (TGTextButton *) gTQSender;
  int ActiveButtonID = ActiveTextButton->WidgetId();
  
  if(!VMEConnectionEstablished)
    return;
  
  // Return if the user has specified that the V1720 digitizer board
  // should be used during this session
  if(!V1720Enable )
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
      // ADAQAcquisition::RunDGScope) during data acquisition
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
      if(ROOTFileOpen){
	if(DGScopeDataStorageEnable_CB->IsDown())
	  DGScopeDataStorageEnable_CB->Clicked();
	
	if(OutputDataFile->IsOpen())
	  DGScopeDataStorageCloseFile_TB->Clicked();
      }
    }
    break;
  }
    
    // Send a software signal to the V1720 to for a manually forced
    // data trigger across all channels
  case DGScopeTrigger_TB_ID:{
    DGManager->SendSWTrigger();
    break;
  }


    // Check to see if the V1720 FPGA buffer is full; the text entry
    // widget will be updated to alert the user
  case DGScopeCheckBufferStatus_TB_ID:{

    bool BufferStatus[NumDataChannels];
    for(int ch=0; ch<NumDataChannels; ch++)
      BufferStatus[ch] = false;
    
    DGManager->CheckBufferStatus(BufferStatus);

    bool BufferFull = false;
    
    for(int ch=0; ch<NumDataChannels; ch++){
      if(BufferStatus[ch] == true)
	BufferFull = true;
    }
    
    if(BufferFull)
      DGScopeBufferStatus_TE->SetText("Buffers are FULL");
    else
      DGScopeBufferStatus_TE->SetText("Buffers are OK");
    
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
      string NewPointLabel = "Calibration point " + ss.str();
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


      DGScopeSpectrumCalibrationCalibrate_TB->SetText("Calibrated");
      DGScopeSpectrumCalibrationCalibrate_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
      DGScopeSpectrumCalibrationCalibrate_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
    }
    break;
  }
    
  case DGScopeSpectrumCalibrationPlot_TB_ID:{
    
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(UseCalibrationManager[CurrentChannel]){
      TCanvas *Calibration_C = new TCanvas("Calibration_C","CalibrationManager TGraph",0,0,600,400);
      Calibration_C->SetLeftMargin(0.14);
      Calibration_C->SetBottomMargin(0.14);
      
      stringstream ss;
      ss << "CalibrationManager TGraph for Channel[" << CurrentChannel << "]";
      string Title = ss.str();

      CalibrationManager[CurrentChannel]->SetTitle(Title.c_str());
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitle("Pulse unit [ADC]");
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleOffset(1.1);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetNdivisions(505);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitle("Energy");
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleOffset(1.2);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->SetMarkerSize(2);
      CalibrationManager[CurrentChannel]->SetMarkerStyle(22);
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
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0", 0);
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
    DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
    DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);

    DGScopeSpectrumCalibrationCalibrate_TB->SetText("Calibrate");
    DGScopeSpectrumCalibrationCalibrate_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
    DGScopeSpectrumCalibrationCalibrate_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
    
    break;
  }

  case DGScopeSpectrumCalibrationLoad_TB_ID:{

    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};

    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &FileInformation);

    if(FileInformation.fFilename==NULL)
      {}

    else{
      
      int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

      string CalibrationFileName = FileInformation.fFilename;

      size_t Found = CalibrationFileName.find_last_of(".");
      if(Found != string::npos){

	// Set the calibration file to an input stream
	ifstream In(CalibrationFileName.c_str());

	// Reset any preexisting calibrations
	DGScopeSpectrumCalibrationReset_TB->Clicked();

	// An index to control the set point
	int SetPoint = 0;

	// Iterate through each line in the file and use the values
	// (column1 == energy, column2 == pulse unit) to set the
	// CalibrationData objects for the current channel
	while(In.good()){
	  double Energy, PulseUnit;
	  In >> Energy >> PulseUnit;

	  if(In.eof()) break;

	  CalibrationData[CurrentChannel].PointID.push_back(SetPoint);
	  CalibrationData[CurrentChannel].Energy.push_back(Energy);
	  CalibrationData[CurrentChannel].PulseUnit.push_back(PulseUnit);

	  // Add a new point to the number of calibration points in case
	  // the user wants to add subsequent points to the calibration
	  stringstream ss;
	  ss << (SetPoint+1);
	  string NewPointLabel = "Calibration point " + ss.str();
	  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(NewPointLabel.c_str(),SetPoint+1);
	  
	  // Set the combo box to display the new calibration point...
	  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(SetPoint+1);

	  SetPoint++;
	}

	// Use the loaded calibration points to set the calibration
	DGScopeSpectrumCalibrationCalibrate_TB->Clicked();
      }
    }
    break;
  }

  case DGScopeSpectrumCalibrationWrite_TB_ID:{
    
    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};
    
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    if(FileInformation.fFilename==NULL)
      {}

    else{
      
      int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

      string CalibrationFileName = FileInformation.fFilename;

      // Set the calibration file to an input stream
      ofstream Out(CalibrationFileName.c_str());
      
      for(int i=0; i<CalibrationData[CurrentChannel].Energy.size(); i++)
	Out << setw(10) << CalibrationData[CurrentChannel].Energy[i]
	    << setw(10) << CalibrationData[CurrentChannel].PulseUnit[i]
	    << endl;
      Out.close();
    }
    break;
  }

    //////////////////////////////
    // Set the ROOT data file name

  case DGScopeDataFileName_TB_ID:{
    
    const char *FileTypes[] = {"ADAQ ROOT file","*.adaq",
			       "ADAQ ROOT file","*.root",
			       0, 0};
    
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fOverwrite = false;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);
    
    if(FileInformation.fFilename==NULL)
      {}
    else{
      DataFileName = FileInformation.fFilename;
      
      size_t Found = DataFileName.find_last_of(".");
      if(Found != string::npos)
	DataFileName = DataFileName.substr(0, Found);

      DataFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];

      Found = DataFileExtension.find_last_of("*");
      DataFileExtension = DataFileExtension.substr(Found+1, DataFileExtension.size());
      string FileName_StripPath = DataFileName + DataFileExtension;

      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());

      DGScopeDataFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    
    break;
  }

    ///////////////////////////
    // Create ROOT data file
  case DGScopeDataStorageCreateFile_TB_ID:{

    ///////////////////////////////////////////////
    // Test to ensure data file is not already open


    /////////////////////////////////////////////
    // Instantiate objects for persistent storage

    string FileName = DataFileName + DataFileExtension;

    // TFile to create a ROOT binary file for output
    OutputDataFile = new TFile(FileName.c_str(), "recreate");
    
    // TTree to store the waveforms as arrays. The array indices are
    // sample numbers and array values are the voltages
    WaveformTree = new TTree("WaveformTree","Prototype tree to store all waveforms of an event");
    
    // TObjString to hold a comment on the measurement data
    MeasComment = new TObjString("Comments are not presently enabled! ZSH 14 Apr 14");
    
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
    for(int ch=0; ch<NumDataChannels; ch++){
      MeasParams->DCOffset.push_back( (int)DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());
      MeasParams->TriggerThreshold.push_back( (int)DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() );
      MeasParams->BaselineCalcMin.push_back( (int)DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber() );
      MeasParams->BaselineCalcMax.push_back( (int)DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber() );
    }
    
    // Retrieve the record length for the acquisition window [samples].
    MeasParams->RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();
    
    // If the user has selected to reduce the output data then modify
    // the record length accordingly. Note that this effectively
    // destroys any pulse timing information, but it presently done to
    // avoid modifying the structure of the ADAQ ROOT files. In the
    // future, this should be correctly implemented. ZSH 26 AUG 13
    if(DGScopeUseDataReduction_CB->IsDown())
      MeasParams->RecordLength /= DGScopeDataReductionFactor_NEL->GetEntry()->GetIntNumber();
    
    ////////////////////////////
    // Set the infamous booleans
    
    // Set a bool indicating that the next digitized event will
    // trigger the creation of a TTree branch with the correctly sized
    // array. This action is performed once in
    // ADAQAcquisition::RunDGScope(). See that function for more comments
    BranchWaveformTree = true;

    ROOTFileOpen = true;
    
    //////////////////////////////////
    // Set widget states appropriately
    
    // Disable the filename, comment, and create file button (since we
    // don't want to create new ROOT files until the active is closed)
    // and activate the close file and enable buttons (since these
    // options are now available with an open ROOT file for data writing)
    //DGScopeDataFileName_TEL->GetEntry()->SetState(false);
    //DGScopeDataComment_TEL->GetEntry()->SetState(false);
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
    DGScopeDataStorageCreateFile_TB->SetText("ADAQ file created");
    DGScopeDataStorageCloseFile_TB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
  
    break;
  }

    ///////////////////////////////
    // Write and close ROOT file
  case DGScopeDataStorageCloseFile_TB_ID:{
    
    if(!ROOTFileOpen)
      break;
    
    if(DGScopeDataStorageEnable_CB->IsDown())
      DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    
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
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
    DGScopeDataStorageCreateFile_TB->SetText("Create ADAQ file");
    DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);

    ROOTFileOpen = false;

    break;
  }

  case DGScopeSpectrumFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"Space-separated format", ".dat",
			       "Comma-separated format", ".csv",
			       0, 0};
    
    // Create a new window containing the file save dialog. Set the
    // default directory to the user's present directory
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    // If a file name was NOT successfully created ...
    if(FileInformation.fFilename==NULL)
      {}
    
    // If a file name was successfully created ...
    else{
      // Get the file name from the text entry widget
      SpectrumFileName = FileInformation.fFilename;
      
      // If the user entered the file name with a dot followed by a
      // file extension, strip the them from the file name since the
      // spectrum file type is assigned via the combo box selection
      // from the file dialog. This ensures that regardless of the
      // user's entry, the right format will prevail!
      size_t Found = SpectrumFileName.find_last_of(".");
      if(Found != string::npos)
	SpectrumFileName = SpectrumFileName.substr(0, Found);
      
      // Get the file type enxtension from the combo box widget (note
      // the "+1" required to get the index right)
      SpectrumFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];
      
      // Get just the file name + file extension by stripping off the
      // absolute path from the full file name

      string FileName_StripPath = SpectrumFileName + SpectrumFileExtension;
      
      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());
      
      DGScopeSpectrumFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    break;
  }
    
  case DGScopeSaveSpectrum_TB_ID:{
    SaveSpectrumData();
    break;
  }
    
  case DGScopeCanvasFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"EPS file", ".eps",
			       "PS file", ".ps",
			       "PDF file", ".pdf",
			       "PNG file", ".png",
			       "JPG file", ".jpeg",
			       0, 0};
    
    // Create a new window containing the file save dialog. Set the
    // default directory to the user's present directory
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    // If a file name was NOT successfully created ...
    if(FileInformation.fFilename==NULL)
      {}//CreateMessageBox("No file name was selected! The canvas graphics will not be saved!","Stop");
    
    // If a file name was successfully created ...
    else{
      // Get the file name from the text entry widget
      GraphicsFileName = FileInformation.fFilename;
      
      // Get the file type enxtension from the combo box widget (note
      // the "+1" required to get the index right)
      GraphicsFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];

      // If the user did not enter the file name with the extension,
      // tack it on to the end
      size_t Found = GraphicsFileName.find_last_of(".");
      if(Found == string::npos)
	GraphicsFileName = GraphicsFileName.substr(0,Found) + GraphicsFileExtension;

      string FileName_StripPath = GraphicsFileName;
      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());
      
      DGScopeCanvasFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    break;
  }
    
    
  case DGScopeSaveCanvas_TB_ID:{
    
    string FileName;
    
    if(DGScopeSaveCanvasWithTimeExtension_CB->IsDown()){
      time_t CurrentTime = time(NULL);
      stringstream ss;
      ss << "." << CurrentTime;
      string CurrentTimeString = ss.str();
      FileName = GraphicsFileName + CurrentTimeString;
    }
    else
      FileName = GraphicsFileName;
    
    DGScope_EC->GetCanvas()->Print(FileName.c_str(), GraphicsFileExtension.c_str());
    break;
  }
    
    
  case DGScopeUpdatePlot_TB_ID:{
    
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(DGScopeUltraHighRate_RB->IsDown())
      break;
    else{
      if(DGScopeSpectrum_H[CurrentChannel])
	ForceSpectrumDrawing();
      break;
    }
  }
    
  case DGScopeAcquisitionTimerStart_TB_ID:{
    
    if(DGScopeStartStop_TB->GetString() != "Acquiring")
      break;
    
    if(ActiveTextButton->GetString() == "Start timer"){
      
      // Set the graphical attributes of the text button
      ActiveTextButton->SetBackgroundColor(ColorManager->Number2Pixel(8));
      ActiveTextButton->SetForegroundColor(ColorManager->Number2Pixel(1));
      ActiveTextButton->SetText("Waiting ...");
      
      // Get the start time (i.e. now)
      AcquisitionTime_Start = time(NULL);

      // Get the stop time (i.e. amount of time to run in seconds)
      AcquisitionTime_Stop = DGScopeAcquisitionTime_NEL->GetEntry()->GetNumber();

      // Set the bool that will trigger the check of the timer against
      // the current time within the acquisition loop in RunDGScope()
      AcquisitionTimerEnabled = true;

      // If the ROOT data file is open but the user has not enabled
      // data storage, assume that the user wants to acquire data for
      // the specific amount of time dictated by the acquisition timer
      if(ROOTFileOpen){
	if(OutputDataFile->IsOpen() and !DGScopeDataStorageEnable_CB->IsDown())
	  DGScopeDataStorageEnable_CB->SetState(kButtonDown);
      }
    }
    break;
  }

  case DGScopeAcquisitionTimerAbort_TB_ID:{
    if(DGScopeEnable && AcquisitionTimerEnabled)
      StopAcquisitionSafely();
    break;
  }
    
  default:
    break;
  }
}


// Perform actions triggers by DGScope number entries
void ADAQAcquisition::HandleScopeNumberEntries()
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
  case DGScopeSpectrumCalibrationEnergy_NEL_ID:
  case DGScopeSpectrumCalibrationPulseUnit_NEL_ID:{
    double Value = 0.;
    if(ActiveEntryID == DGScopeSpectrumCalibrationEnergy_NEL_ID)
      Value = DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();
    else
      Value = DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    
    // Normalize value for slider position setting from 0 to 1
    Value /= DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();
    
    DGScopeHorizontalScale_THS->SetPointerPosition(Value);
    break;
  }
  }
}

void ADAQAcquisition::HandleRadioButtons()
{
  TGRadioButton *ActiveRadioButton = (TGRadioButton *) gTQSender;
  int RadioButtonID = ActiveRadioButton->WidgetId();

  switch(RadioButtonID){

  case DGScopeSpectrumAnalysisHeight_RB_ID:
    
    if(DGScopeSpectrumAnalysisHeight_RB->IsDown()){
      DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(0);
      DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(4095);
    }

    break;

  case DGScopeSpectrumAnalysisArea_RB_ID:
    
    if(DGScopeSpectrumAnalysisArea_RB->IsDown()){
      DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(4000);
      DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(100000);
    }
    
    break;
    
  default:
    break;
  }
}


void ADAQAcquisition::HandleComboBoxes(int ComboBoxID, int SelectedID)
{
  switch(ComboBoxID){

  case DGScopeSpectrumChannel_CBL_ID:{

    // In order to ensure that the calibration set point combo box
    // displays the correct number of calibration points, we need to
    // rebuild the combo box entry list each time the user selects a
    // new histogram channel to plot using this combo box

    // Get the number of calibration points for the current histogram
    // (SelectedID == histogram channel for this combo box)
    int LastPointIndex = CalibrationData[SelectedID].PointID.size();

    // Remove the previous entries from the combo box
    int TotalEntries = DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->GetNumberOfEntries();
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->RemoveEntries(0,TotalEntries-1);

    // Recreate the entries for each calibration point in the present
    // histogram calibration combo box
    stringstream ss;
    for(int i=0; i<=LastPointIndex; i++){
      ss << "Calibration point " << i;
      string Entry = ss.str();
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(Entry.c_str(), i);
      ss.str("");
    }

    // If the current histogram has calibration points then set the
    // combo box to diplay the to-be-set point
    if(LastPointIndex > 0){
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(LastPointIndex);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }

    // ... otherwise display the no-calibration points values
    else{
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(0);
    }
    break;
  }

  case DGScopeSpectrumCalibrationPoint_CBL_ID:{

    int SpectrumChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();    
    int Size = CalibrationData[SpectrumChannel].PointID.size();

    if(SelectedID <= (Size-1)){

      double Energy = CalibrationData[SpectrumChannel].Energy.at(SelectedID);
      double PulseUnit = CalibrationData[SpectrumChannel].PulseUnit.at(SelectedID);

      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(Energy);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(PulseUnit);

      double Value = Energy/DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();

      DGScopeHorizontalScale_THS->SetPointerPosition(Value);
    }
    else{
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }
    break;
  }
  }
}
  

void ADAQAcquisition::HandleCheckButtons()
{
  TGCheckButton *ActiveCheckButton = (TGCheckButton *) gTQSender;
  int CheckButtonID = ActiveCheckButton->WidgetId();
  
  switch(CheckButtonID){
    
    // Enable the calibration widgets
  case DGScopeSpectrumCalibration_CB_ID:
    if(DGScopeSpectrumCalibration_CB->IsDown()){
      DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(true);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(true);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(true);
      DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationReset_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationLoad_TB->SetState(kButtonUp);
      DGScopeSpectrumCalibrationWrite_TB->SetState(kButtonUp);
    }

    // Disable the calibration widgets
    else{
      DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(false);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
      DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationReset_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationLoad_TB->SetState(kButtonDisabled);
      DGScopeSpectrumCalibrationWrite_TB->SetState(kButtonDisabled);
    }
    break;
  }
}


// Performm actions that ensure a safe shutdown and disconnect of the
// ADAQAcquisition software from the VME boards
void ADAQAcquisition::HandleDisconnectAndTerminate(bool TerminateApplication)
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

  if(V1718Enable)
    BRManager->CloseLink();

  // Close the standalone ROOT application
  if(TerminateApplication)
    gApplication->Terminate();
}


// Set the state of the HV voltage and maximum current entry
// widgets. When a specific channel is turned "on", it's HV and I
// setting widgets are turned "off"
void ADAQAcquisition::SetHVWidgetState(int HVChannel, bool HVActive)
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
void ADAQAcquisition::SetDGWidgetState(bool AcquiringData)
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
    DGScopeZSThreshold_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGScopeZSSamples_NEL[ch]->GetEntry()->SetState(WidgetState);
  }

  DGScopeTriggerCoincidenceEnable_CB->SetState(ButtonState,true);
  DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(WidgetState);
  DGScopeTriggerMode_CBL->GetComboBox()->SetEnabled(WidgetState);

  DGScopeRecordLength_NEL->GetEntry()->SetState(WidgetState);
  DGScopePostTriggerSize_NEL->GetEntry()->SetState(WidgetState);

  DGScopeWaveform_RB->SetEnabled(WidgetState);
  DGScopeSpectrum_RB->SetEnabled(WidgetState);
  DGScopeHighRate_RB->SetEnabled(WidgetState);
  DGScopeUltraHighRate_RB->SetEnabled(WidgetState);

  DGScopeMaxEventsBeforeTransfer_NEL->GetEntry()->SetState(WidgetState);
  DGScopeDataReductionFactor_NEL->GetEntry()->SetState(WidgetState);

  DGScopeZSMode_CBL->GetComboBox()->SetEnabled(WidgetState);

  DGScopeSpectrumBinNumber_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumMinBin_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumMaxBin_NEL->GetEntry()->SetState(WidgetState);

  DGScopeSpectrumAnalysisHeight_RB->SetEnabled(WidgetState);
  DGScopeSpectrumAnalysisArea_RB->SetEnabled(WidgetState);
  DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetState(WidgetState);

  DGScopeSpectrumAggregateRuns_CB->SetState(ButtonState,true);
  
  if(AcquiringData)
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
  else
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);

  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetState(WidgetState);
}


// Run the real-time updating of the ROOT number entry widgets that
// display active voltage and drawn current from all channels
void ADAQAcquisition::RunHVMonitoring()
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


void ADAQAcquisition::RunDGScope()
{
  /////////////////////////////////////////////////
  // Initialize local and class member variables //
  /////////////////////////////////////////////////

  /////////////////////////////////////////
  // Variables for V1720 digitizer settings

  // Get the record length, ie, number of 4ns samples in acquisition window
  uint32_t RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();

  // Get the data thinning factor 
  bool UseDataReduction = DGScopeUseDataReduction_CB->IsDown();
  uint32_t DataReductionFactor = DGScopeDataReductionFactor_NEL->GetEntry()->GetIntNumber();

  if(RecordLength % DataReductionFactor != 0 && UseDataReduction){
    cout << "\nError! (RecordLength % DataReductionFactor) MUST equal zero to avoid grevious errors!\n"
	 <<   "       Adjust the data reduction factor and restart acquisition ...\n"
	 << endl;

    StopAcquisitionSafely();
  }
  
  // Get the percentage of acquisition window that occurs after the trigger
  uint32_t PostTriggerSize = DGScopePostTriggerSize_NEL->GetEntry()->GetIntNumber();

  // Variables for graphing the digitized waveforms as time versus
  // voltage. Units are determined by the user's selections when the
  // arrays are filled inside the acquisition loop
  double Time_graph[RecordLength], Voltage_graph[RecordLength]; 

  // The following variables must be set / composed of values from
  // all 8 digitzer channels. Declare them and then initialize.
   
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

  for(int ch=0; ch<NumDataChannels; ch++){

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
  // enabled bit mask; if not, alert the user and return without
  // beginning acquisition since there ain't nothin' to acquire.
  if((0xff & ChannelEnableMask)==0){
    {}//CreateMessageBox("At least one V1720 digitizer channel must be enabled before acquisition will initiate!","Stop");
    return;
  }
   
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
  double minBin = DGScopeSpectrumMinBin_NEL->GetEntry()->GetNumber();
  double maxBin = DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();
   
  // Allocate variables for the X and Y minimum and maximum positions
  // of the X and Y double sliders that control canvas plotting range
  float xMin, xMax, yMin, yMax;
   
  // Get the bools to determine what (if anything) is plotted
  bool PlotWaveform = DGScopeWaveform_RB->IsDown();
  bool PlotSpectrum = DGScopeSpectrum_RB->IsDown();
  //bool HighRate = DGScopeHighRate_RB->IsDown();
  bool UltraHighRate = DGScopeUltraHighRate_RB->IsDown();
   
  // Get the bools to determine plotting options
  bool DrawLegend = DGScopeDisplayDrawLegend_CB->IsDown();
   
  // Get the plotting units and set conversion units accordingly
  bool PlotXAxisInSamples = DGScopeDisplayWaveformXAxisSample_RB->IsDown();
  bool PlotYAxisInADC = DGScopeDisplayWaveformYAxisADC_RB->IsDown();

  double ConvertTimeToGraphUnits = DGManager->GetNanosecondsPerSample(); 
  if(PlotXAxisInSamples)
    ConvertTimeToGraphUnits = 1.;
   
  double ConvertVoltageToGraphUnits = DGManager->GetMillivoltsPerBit();
  if(PlotYAxisInADC)
    ConvertVoltageToGraphUnits = 1.;
   
  // Get the signal polarity setting for each channel
  double SignalPolarity[NumDataChannels];
  for(int ch=0; ch<NumDataChannels; ch++){
    if(DGScopeChannelPosPolarity_RB[ch]->IsDown())
      SignalPolarity[ch] = 1.0;
    else
      SignalPolarity[ch] = -1.0;
  }
   
  // The following variables are declared here but used/set within
  // the acquisition loop

  // Variables for plotted waveform vertical offset and plotted
  // channel trigger threshold. Note that these variables are set
  // dynamicallly within the acquisition loop to allow the user
  // flexibility in setting these values
  double VertPosOffset, ChTrigThr;
  VertPosOffset = ChTrigThr = 0;

  // Variables for baseline calculation. These variables are used
  // within the acquisition loop.
  double BaseCalcMin, BaseCalcMax, BaseCalcResult;
  BaseCalcMin = BaseCalcMax = BaseCalcResult = 0;

  // Variables for waveform sample height and channel trigger
  // threshold hold height above the baseline
  double SampleHeight = 0.;
  double TriggerHeight = 0.;

  // Variables contain the pulse height and integrated pulse area
  // values. Units are in [ADC] until transformed by calibration
  // factor into units of [keV]
  double PulseHeight = 0.;
  double PulseArea = 0.;
   
  // The active channel to histogram
  uint32_t ChannelToHistogram = 0;

  // Bools to determine whether the incoming waveforms are filted into
  // a spectrum by pulse height (maximum value of voltage above the
  // baseline during the entire record length) or by pulse area
  // (integrated area between the baseline and the voltage trace)
  bool AnalyzePulseHeight = DGScopeSpectrumAnalysisHeight_RB->IsDown();
  bool AnalyzePulseArea = DGScopeSpectrumAnalysisArea_RB->IsDown();

  // Lower/Upper threshold values for adding a pulse to the pulse
  // spectrum histogram / output to ROOT TFile
  int LowerLevelDiscr = DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->GetIntNumber();
  int UpperLevelDiscr = DGScopeSpectrumAnalysisULD_NEL->GetEntry()->GetIntNumber();
  bool DiscrOKForOutput = false;
   
  // Iterate through all 8 digitizer channels
  for(int ch=0; ch<NumDataChannels; ch++){
     
    // Initialize pulse spectrum histograms for each channel,
    // deleting the previous ones if they exist to prevent memory
    // leaks. Assign the correct graphical styles as necessary.
    if(DGScopeSpectrum_H[ch]) delete DGScopeSpectrum_H[ch];
    DGScopeSpectrum_H[ch] = new TH1F("","",bins,minBin,maxBin);
    DGScopeSpectrum_H[ch]->SetLineWidth(2);
    DGScopeSpectrum_H[ch]->SetLineColor(ch+1);
  }

  // Set the style of the histogram statistics box
  gStyle->SetOptStat("ne");
   
  // Convert the time to the user's desired units for graphing
 // the waveform [sample or ns]
  for(uint32_t sample=0; sample<RecordLength; sample++)
    Time_graph[sample] = sample * ConvertTimeToGraphUnits;

  // Assign values to use if built in debug mode (Not used)
  /*
  bool DebugModeEnabled = false;
  int DebugModeWaveformGenerationPause = 1000;
  if(BuildInDebugMode){
    DebugModeEnabled = DebugModeEnable_CB->IsDown();
    DebugModeWaveformGenerationPause = DebugModeWaveformGenerationPause_NEL->GetEntry()->GetIntNumber();
  }
  */

  // Assign the frequency (in number of histogram entries) with which
  // the canvas will be updated
  int SpectrumRefreshRate = DGScopeSpectrumRefreshRate_NEL->GetEntry()->GetIntNumber();

  // Assign values to be used with the acquisition timer.
  time_t AcquisitionTime_Now, AcquisitionTime_Prev;
  AcquisitionTime_Now = AcquisitionTime_Prev = 0;


  //////////////////////////////////////////////
  // Prepare the ROOT output file and objects //
  //////////////////////////////////////////////

  // Define a vector of vectors that will hold the digitized waveforms
  // in all channels (units of [ADC]). The outer vector (size 8)
  // represents each digitizer channel; the inner vector (size
  // RecordLength) represents the waveform. The start address of each
  // outer vector will be used to create a unique branch in the
  // waveform TTree object to store each of the 8 digitizer channels 
  vector<vector<int> > Voltage;
  vector<vector<int> > VoltageTmp;

  uint32_t factor = RecordLength / DataReductionFactor;

  // Resize the outer and inner vector to the appropriate, fixed size
  Voltage.resize(NumDataChannels);
  VoltageTmp.resize(NumDataChannels);
  for(int ch=0; ch<NumDataChannels; ch++){
    if(DGScopeChannelEnable_CB[ch]->IsDisabledAndSelected()){
      Voltage[ch].resize(RecordLength);
      VoltageTmp[ch].resize(factor);
    }
    else{
      Voltage[ch].resize(0);
      VoltageTmp[ch].resize(0);
    }
  }
  

  ///////////////////////////////////////////////////////
  // Program V1720 digitizer with acquisition settings //
  ///////////////////////////////////////////////////////

  ///////////////////////////////////////////////
  // Variables for digitizer readout

  // CAEN_DGTZ type variables for readout of the digitized waveforms
  // from the V1720 FPGA buffer onto the PC buffer
  char *EventPointer = NULL;
  CAEN_DGTZ_EventInfo_t EventInfo;
  CAEN_DGTZ_UINT16_EVENT_t *EventWaveform = NULL;

  uint32_t FPGAEventAddress = 0x812c;
  uint32_t NumFPGAEvents = 0;

  // Variables for PC buffer
  char *Buffer = NULL;
  uint32_t BufferSize;
  uint32_t Size, NumEvents;

  // Reset the digitizer to default state
  DGManager->Reset();

  // Set the trigger threshold, DC offsets, and ZS parameters
  // individually for each of the 8 digitizer channels
  for(int ch=0; ch<NumDataChannels; ch++){
    DGManager->SetChannelTriggerThreshold(ch,ChannelTriggerThreshold[ch]);
    DGManager->SetChannelDCOffset(ch, DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());

    int32_t Threshold = DGScopeZSThreshold_NEL[ch]->GetEntry()->GetIntNumber();
    int32_t Samples = DGScopeZSSamples_NEL[ch]->GetEntry()->GetIntNumber();
    DGManager->SetChannelZSParams(ch, 0, Threshold, Samples);
  }

  // Set the trigger mode
  switch(TriggerMode){

    // Mode: External trigger (NIM logic input signal)
  case 0:{
    DGManager->SetExtTriggerInputMode(CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    DGManager->SetChannelSelfTrigger(CAEN_DGTZ_TRGMODE_DISABLED, ChannelEnableMask);
    DGManager->SetSWTriggerMode(CAEN_DGTZ_TRGMODE_DISABLED);

    // Get the value of the front panel I/O control register
    uint32_t FrontPanelIOControlRegister = 0x811C;
    uint32_t FrontPanelIOControlValue = 0;
    DGManager->GetRegisterValue(FrontPanelIOControlRegister, &FrontPanelIOControlValue);

    // When Bit[0] of 0x811C == 0, NIM logic is used for input; so
    // clear Bit[0] using bitwise ops
    FrontPanelIOControlValue &= ~(1<<0);
    DGManager->SetRegisterValue(FrontPanelIOControlRegister, FrontPanelIOControlValue);

    break;
  }

    // Mode: External trigger (TTL logic input signal)
  case 1:{
    DGManager->SetExtTriggerInputMode(CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    DGManager->SetChannelSelfTrigger(CAEN_DGTZ_TRGMODE_DISABLED, ChannelEnableMask);
    DGManager->SetSWTriggerMode(CAEN_DGTZ_TRGMODE_DISABLED);

    // Get the value of the front panel I/O control register
    uint32_t FrontPanelIOControlRegister = 0x811C;
    uint32_t FrontPanelIOControlValue = 0;
    DGManager->GetRegisterValue(FrontPanelIOControlRegister, &FrontPanelIOControlValue);

    // When Bit[0] of 0x811C == 1, TTL logic is used for input; so set
    // Bit[0] using bitwise ops
    FrontPanelIOControlValue |= 1<<0;
    DGManager->SetRegisterValue(FrontPanelIOControlRegister, FrontPanelIOControlValue);
  }

    // Mode: Automatic channel threshold triggering
  case 2:
    DGManager->SetExtTriggerInputMode(CAEN_DGTZ_TRGMODE_DISABLED);
    DGManager->SetChannelSelfTrigger(CAEN_DGTZ_TRGMODE_ACQ_ONLY, ChannelEnableMask);
    DGManager->SetSWTriggerMode(CAEN_DGTZ_TRGMODE_DISABLED);
    break;

    // Mode: Software trigger
  case 3:
    DGManager->SetExtTriggerInputMode(CAEN_DGTZ_TRGMODE_DISABLED);
    DGManager->SetChannelSelfTrigger(CAEN_DGTZ_TRGMODE_DISABLED, ChannelEnableMask);
    DGManager->SetSWTriggerMode(CAEN_DGTZ_TRGMODE_ACQ_ONLY);
    break;
  }

  // Set the record length of the acquisition window
  DGManager->SetRecordLength(RecordLength);

  // Set the channel enable mask
  DGManager->SetChannelEnableMask(ChannelEnableMask);

  // Set the Zero-Supperssion mode
  DGManager->SetZeroSuppressionMode(DGScopeZSMode_CBL->GetComboBox()->GetSelected());

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
  uint32_t MaxEvents = DGScopeMaxEventsBeforeTransfer_NEL->GetEntry()->GetIntNumber();
  DGManager->SetMaxNumEventsBLT(MaxEvents);

  // Allocate memory for the readout buffer
  DGManager->MallocReadoutBuffer(&Buffer, &Size);

  // Set the percentage of acquisition window that occurs after trigger
  DGManager->SetPostTriggerSize(PostTriggerSize);

  //timespec preA, postA;

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
  // DGScopeEnable bool is true (see CyDAQRootGUI::HandleScopeTextButtons)
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
    if(PlotWaveform or PlotSpectrum){
      DGScopeHorizontalScale_THS->GetPosition(&xMin, &xMax);
      DGScopeVerticalScale_DVS->GetPosition(&yMin, &yMax);
      
      // The value of ConvertTimeToGraphUnits is set before the
      // acquisition loop begins: if plotting the X axis in samples,
      // value == 1.; if plotting the X axis in nanoseconds value == 4
      xMin *= (RecordLength * ConvertTimeToGraphUnits);
      xMax *= (RecordLength * ConvertTimeToGraphUnits);
      
      // The value of ConvertVoltageToGraphUnits is set before the
      // acquisition loop begins: if plotting the Y axis in ADC units,
      // value == 1.; if plotting the Y axis in millivolts value ==
      // (2000./4096)
      yMin *= DGManager->GetMaxBit()*ConvertVoltageToGraphUnits;
      yMax *= DGManager->GetMaxBit()*ConvertVoltageToGraphUnits;
    }
    

    /////////////////////
    // Begin data readout

    // Get number of events in FPGA buffer
    DGManager->GetRegisterValue(FPGAEventAddress, &NumFPGAEvents);

    // To reduce overhead, only readout the FPGA buffer when it has
    // reached the user-set max events
    if(NumFPGAEvents < MaxEvents)
      continue;
    
    // Read data from the V1720 buffer into the PC buffer
    DGManager->ReadData(Buffer, &BufferSize);    

    // Determine the number of events in the buffer

    DGManager->GetNumEvents(Buffer, BufferSize, &NumEvents);

    // For each event in the PC memory buffer...
    for(uint32_t evt=0; evt<NumEvents; evt++){
      
      // Get the event information
      DGManager->GetEventInfo(Buffer, BufferSize, evt, &EventInfo, &EventPointer);

      // Decode the event and obtain the waveform (voltage as a function of time)
      DGManager->DecodeEvent(EventPointer, &EventWaveform);

      // If there is no waveform in the PC buffer, continue in the
      // while loop to avoid segfaulting
      if(EventWaveform==NULL)
	continue;

      // If the user has enabled the acquisition timer ...
      if(AcquisitionTimerEnabled){

	// Calculate the time that has elapsed since the start of
	// acquisition loop
	AcquisitionTime_Prev = AcquisitionTime_Now;
	AcquisitionTime_Now = time(NULL) - AcquisitionTime_Start; // [seconds]
	
	// Update the ROOT widget only every second to notify the user
	// of progress in the countdown. Note that the
	// AcquisitionTime_* variables are integers. Therefore, this
	// conditional will only be satisfied right when the second
	// ticks over and "_Prev" time is different from"_Now" time
	// for only a single pass of the while loop. This should
	// maximize efficiency of the acquisition loop.
	if(AcquisitionTime_Prev != AcquisitionTime_Now){
	  int Countdown = AcquisitionTime_Stop - AcquisitionTime_Now;
	  DGScopeAcquisitionTimer_NEFL->GetEntry()->SetNumber(Countdown);
	}
	
	// If the timer has reached zero, i.e. the acquisition loop
	// has been running for the during specified by the user then
	// turn the acquisition off, which requires resetting a few
	// variables and ROOT widgets
	if(AcquisitionTime_Now >= AcquisitionTime_Stop)
	  StopAcquisitionSafely();
      }
      
      // For each channel...
      for(int ch=0; ch<NumDataChannels; ch++){
	
	// Only proceed to waveform analysis if the channel is enabled
	if(!DGScopeChannelEnable_CB[ch]->IsDisabledAndSelected())
	  continue;
	
	// Initialize the pulse height and pulse area variables
	PulseHeight = PulseArea = 0.; // [ADC]

	// Reset all channel baseline before each event
	BaselineCalcResult[ch] = 0.;

	// For all of the samples in the acquisition window of length RecordLength...
	for(uint32_t sample=0; sample<RecordLength; sample++){
	  
	  // Readout the pulse into the storage vector
	  Voltage[ch][sample] = EventWaveform->DataChannel[ch][sample]; // [ADC]
	  
	  if(UseDataReduction && (sample % DataReductionFactor == 0)){
	    const int Index = sample / DataReductionFactor;
	    VoltageTmp[ch][Index] = EventWaveform->DataChannel[ch][sample]; // [ADC]
	  }
	  
	  // Do not perform the following for ultrahigh rate readout
	  if(!UltraHighRate){

	    // Convert the voltage [ADC] into suitable form for graphing
	    // (accounts for units of ADC/mV and vertical offset)
	    if(PlotWaveform)
	      Voltage_graph[sample] = (Voltage[ch][sample] + DGScopeVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber()) * ConvertVoltageToGraphUnits;
	    
	    // Calculate the baseline by taking the average of all
	    // samples that fall within the baseline calculation region
	    if(sample > BaselineCalcMin[ch] and sample < BaselineCalcMax[ch])
	      BaselineCalcResult[ch] += Voltage[ch][sample]*1.0/(BaselineCalcLength[ch]-1); // [ADC]
	    
	    // Analyze the pulses to obtain pulse spectra
	    else if(sample >= BaselineCalcMax[ch]){
	      
	      // Calculate the voltage ("height") of the sample above
	      // the baseline, using the signal polarity to ensure SampleHeight>0
	      SampleHeight = SignalPolarity[ch] * (Voltage[ch][sample] - BaselineCalcResult[ch]);
	      TriggerHeight = SignalPolarity[ch] * (DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() - BaselineCalcResult[ch]);
	      
	      // Simple algorithm to determine maximum peak height in the pulse
	      if(SampleHeight > PulseHeight and SampleHeight > TriggerHeight)
		PulseHeight = SampleHeight;
	      
	      // Integrate the area under the pulse
	      if(SampleHeight > TriggerHeight)
		PulseArea += SampleHeight;
	    }
	  }
	}
	
	if(!UltraHighRate){
	
	  // If a CalibrationManager (really, a ROOT TGraph) exists, ie,
	  // has been successfully created and is valid for
	  // interpolation then convert PulseHeight/Area
	  if(UseCalibrationManager[ch]){
	    // Use the ROOT TGraph CalibrationManager to convert the
	    // pulse height/area from ADC to keV using LINEAR
	    // interpolation on the pre-assigned calibration points
	    if(AnalyzePulseHeight)
	      PulseHeight = CalibrationManager[ch]->Eval(PulseHeight);
	    else
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
	  
	  if(AnalyzePulseHeight){
	    if(PulseHeight>LowerLevelDiscr and PulseHeight<UpperLevelDiscr)
	      DGScopeSpectrum_H[ch]->Fill(PulseHeight);
	    
	    if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and
	       ch == DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->GetSelected())
	      DiscrOKForOutput = true;
	  }
	  
	  else if(AnalyzePulseArea){
	    if(PulseArea>LowerLevelDiscr and PulseArea<UpperLevelDiscr){
	      DGScopeSpectrum_H[ch]->Fill(PulseArea);
	      
	      if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and 
		 ch == DGScopeSpectrumAnalysisLDTriggerChannel_CBL->GetComboBox()->GetSelected())
		DiscrOKForOutput = true;
	    }
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
	// CyDAQRootGUI::HandleScopeButtons, which precludes creating
	// the branch there since the fixed-length array variable
	// "Voltage" must be set in this member function at the
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
	  for(Int_t channel=0; channel<NumDataChannels; channel++){
	    
	    // ...create a channel-specific name string...
	    ss << "VoltageInADC_Ch" << channel;
	    string WaveformTreeBranchName = ss.str();
	    ss.str("");

	    // ...and use it to specify a channel-specific branch in
	    /// the waveform TTree The branch holds the address of the
	    // vector that contains the waveform as a function of
	    // record length and the RecordLength of each waveform
	    
	    if(UseDataReduction)
	      WaveformTree->Branch(WaveformTreeBranchName.c_str(), 
				   &VoltageTmp[channel]);
	    else
	      WaveformTree->Branch(WaveformTreeBranchName.c_str(), 
				   &Voltage[channel]);
	  }
	  BranchWaveformTree = false;
	}
	
	// Plot the digitized waveforms in 'oscilloscope' mode
	if(PlotWaveform){
	  
	  // Waveform plotting requires small but non-significant
	  // CPU/graphics processing. If the waveform acquisition
	  // rate, especially with multiple channels enabled, is
	  // extremely high, waveform plotting will lag behind event
	  // readout rates, causing the V1720 buffer to overflow and
	  // the ROOT GUI to become sluggish. Because the waveform
	  // plotting mode is only to be used for detector inspection,
	  // tuning acquisition settings, testing, and so on (rather
	  // than high-rate data acquistion, it is not necessary to
	  // plot every single readout event; therefore, I have
	  // limited the plotting to only the first event in a given
	  // V1720 buffer dump.
	  
	  if(evt == 0){
	    
	    DGScope_EC->GetCanvas()->SetLogx(false);
	    DGScope_EC->GetCanvas()->SetLogy(false);
	    
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
	      DGScopeWaveform_G[ch]->Draw("AL");
	      
	      if(DrawLegend)
		DGScopeWaveform_Leg->Draw();
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
	    
	    DGScopeChannelTrigger_L[ch]->DrawLine(xMin, ChTrigThr, xMax, ChTrigThr);
	    
	    // Draw a shaded box region to represent the area of the
	    // waveform being used to calculate the current baseline for
	    // each digitized waveform
	    BaseCalcMin = BaselineCalcMin[ch]*ConvertTimeToGraphUnits;
	    BaseCalcMax = BaselineCalcMax[ch]*ConvertTimeToGraphUnits;
	    BaseCalcResult = (BaselineCalcResult[ch] + VertPosOffset) * ConvertVoltageToGraphUnits;

	    DGScopeBaselineCalcRegion_B[ch]->DrawBox(BaseCalcMin, (BaseCalcResult-100), BaseCalcMax, (BaseCalcResult+100));
	  }
	}
	
	// Plot the integrated pulses in 'multichannel analyzer' mode
	else if(PlotSpectrum){

	  // Determine the channel desired for histogramming into a pulse height spectrum
	  ChannelToHistogram = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
	  
	  // Update the spectrum when specified by the user
	  if(int(DGScopeSpectrum_H[ChannelToHistogram]->GetEntries())%SpectrumRefreshRate==0){
	    
	    // Need to get the raw positions of the sliders again at
	    // present since variables are transformed automatically
	    // for waveform plotting at top of acquisition loop
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
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetRangeUser(xMin,xMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetRangeUser(yMin,yMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->Draw("L");
	    
	    // If the calibration mode is enabled, then draw the third
	    // slider position from the horizontal triple slider and
	    // use its position to calculate the correct calibration
	    // factor. Update the calibration factor number entry
	    // widget with the calculated value

	    // If the user has enabled spectrum calibration mode ...
	    if(DGScopeSpectrumCalibration_CB->IsDown()){
	      
	      double LinePosX;

	      // If the user has enabled the use of the calibration
	      // slider (the "pointer" on the horizontal triple slider
	      // widget located underneath the canvas) ...
	      if(DGScopeSpectrumUseCalibrationSlider_CB->IsDown()){
		
		// Get the pointer position
		LinePosX = DGScopeHorizontalScale_THS->GetPointerPosition()*maxBin;
		
		// Set the pointer position (in correct units) to the
		// ROOT GUI widget that displays the pulse unit
		DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(DGScopeHorizontalScale_THS->GetPointerPosition() * maxBin);
	      }
	      
	      // ... otherwise, allow the user to set the pulse unit manually
	      else
		LinePosX = DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
	      
	      // Draw the vertical calibration line
	      DGScopeSpectrumCalibration_L->DrawLine(LinePosX, yMin, LinePosX, yMax);
	    }
	    
	    // Update the canvas
	    DGScope_EC->GetCanvas()->Update();
	  }
	}
	
	// Do not plot anything. This mode is most useful to
	// maximizing the data throughput rate.
	else{
	}
      }
      
      // After all the channels in the event have been iterated
      // through to extract the waveforms, store the digitized
      // waveforms for all channels in the ROOT TTree object provided:
      // 0. the waveform tree has been created 
      // 1. the user wants to dump the data to a ROOT file
      if(WaveformTree and DGScopeDataStorageEnable_CB->IsDown()){
	
	// If the user has specified that the LLD/ULD should be used
	// as the "trigger" (for plotting the PAS/PHS and writing to a
	// ROOT file) but the present waveform is NOT within the
	// LLD/ULD window (indicated by the DiscrOKForOutput bool set
	// above during analysis of the readout waveform then do NOT
	// write the waveform to the ROOT TTree
	if(DGScopeSpectrumAnalysisLDTrigger_CB->IsDown() and !DiscrOKForOutput)
	  continue;
	
	// Fill the TTree
	//clock_gettime(CLOCK_REALTIME, &preA);
	WaveformTree->Fill();
	//clock_gettime(CLOCK_REALTIME, &postA);
	
	// Reset the bool used to determine if the LLD/ULD window
	// should be used as the "trigger" for writing waveforms
	DiscrOKForOutput = false;
      }
      
      // Update the TGraph with the waveforms
      if(PlotWaveform)
	DGScope_EC->GetCanvas()->Update();
      
      DGManager->FreeEvent(&EventWaveform);
    }

    /*
    long elapsedA_sec = preA.tv_sec - postA.tv_sec;
    long elapsedA_nsec = 0;
    if(postA.tv_nsec > preA.tv_nsec)
      elapsedA_nsec = postA.tv_nsec - preA.tv_nsec;
    else
      elapsedA_nsec = (1e9 + postA.tv_nsec) - preA.tv_nsec;
    
    cout << std::dec 
	 << "Elapsed time: " << elapsedA_sec << "s \t " << elapsedA_nsec << "ns" 
	 << endl;
    */
  } // End DGScope acquisition 'while' loop
  
  // Once the acquisition session has concluded, free the memory that
  // was allocated to the V1720 and PC event buffers
  DGManager->FreeReadoutBuffer(&Buffer);
}


void ADAQAcquisition::SaveSpectrumData()
{
  // Get the current channel
  int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

  //Ensure that the spectrum histogram object exists
  if(!DGScopeSpectrum_H[CurrentChannel]){
    //    CreateMessageBox("The Spectrum_H object does not yet exist so there is nothing to save to file!","Stop");
    return;
  }
  else{
    if(SpectrumFileExtension == ".dat" or SpectrumFileExtension == ".csv"){
      
      // Create the full file name (name + extension)
      string FName;
      
      if(DGScopeSaveSpectrumWithTimeExtension_CB->IsDown()){
	time_t CurrentTime = time(NULL);
	stringstream ss;
	ss << "." << CurrentTime;
	string CurrentTimeString = ss.str();
	FName = SpectrumFileName + SpectrumFileExtension + CurrentTimeString;
      }
      else
	FName = SpectrumFileName + SpectrumFileExtension;
      
      // Create an ofstream object to write the data to a file
      ofstream SpectrumOutput(FName.c_str(), ofstream::trunc);
      
      // Assign the data separator based on file extension
      string separator;
      if(SpectrumFileExtension == ".dat")
	separator = "\t";
      else if(SpectrumFileExtension == ".csv")
	separator = ",";
      
      // Get the number of bins in the spectrum histogram
      int Bins = DGScopeSpectrum_H[CurrentChannel]->GetNbinsX();
      
      // Iterate over all the bins in spectrum histogram and output
      // the bin center (value on the X axis of the histogram) and the
      // bin content (value on the Y axis of the histogram)
      for(int i=1; i<=Bins; i++){
	double BinCenter = DGScopeSpectrum_H[CurrentChannel]->GetBinCenter(i);
	double BinContent = DGScopeSpectrum_H[CurrentChannel]->GetBinContent(i);
	
	SpectrumOutput << BinCenter << separator << BinContent
		       << endl;
      }
      
      // Close the ofstream object
      SpectrumOutput.close();
    }
    else{
      //CreateMessageBox("Unacceptable file extension for the spectrum data file! Valid extensions are '.dat' and '.csv'!","Stop");
      return;
    }
  }
}


// Method that enables the user to force the drawing of the pulse
// spectrum histogram for the specified channel. This can be used
// while acquiring data (most importantly when the user is acquiring
// data in high-throughput mode where nothing is plotted by default,
// he/she can force plotting to evaluate the progress/validity of the
// spectrum without slowing down acquisition) or after the acquisition
// is complete and turned off (setting titles, positions, etc) for
// pretty output to file.
void ADAQAcquisition::ForceSpectrumDrawing()
{
  int ChannelToHistogram = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
  
  float xMin, xMax, yMin, yMax;
  int maxBin = DGScopeSpectrumMaxBin_NEL->GetEntry()->GetIntNumber();
  
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
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->CenterTitle();
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetRangeUser(xMin,xMax);
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->CenterTitle();
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetRangeUser(yMin,yMax);
  DGScopeSpectrum_H[ChannelToHistogram]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
  DGScopeSpectrum_H[ChannelToHistogram]->Draw("L");

  DGScope_EC->GetCanvas()->Update();
}


// Method to safely cease acquiring data, including writing and
// closing of possibly opened ROOT files.
void ADAQAcquisition::StopAcquisitionSafely()
{
  // Stop the V1720 from acquiring data first thing
  DGManager->SWStopAcquisition();
  
  // Update button color from green to red and update text to "Start"
  DGScopeStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  DGScopeStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
  DGScopeStartStop_TB->SetText("Stopped");
  
  // Set bool to disable the DGScope loop
  DGScopeEnable = false;
  
  // Set the appropriate state for the relevant widgets
  SetDGWidgetState(false);
  
  // Determine if a ROOT file was open and receiving data when the
  // user clicked to stop acquiring data; if so, ensure that the
  // data is written and the ROOT file is properly closed before
  // resetting widget state to prevent seg faults and that ROOT
  // file properly written.
  if(ROOTFileOpen){
    if(DGScopeDataStorageEnable_CB->IsDown())
      DGScopeDataStorageEnable_CB->Clicked();
    
    if(OutputDataFile->IsOpen())
      DGScopeDataStorageCloseFile_TB->Clicked();
  }
  
  // Determine if the acquisition timer is active
  if(AcquisitionTimerEnabled){
    // Reset the attributes of the timer start text button
    DGScopeAcquisitionTimerStart_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
    DGScopeAcquisitionTimerStart_TB->SetText("Start timer");
    
    // Set the acquisition enabled boolean to false
    AcquisitionTimerEnabled = false;

    // When the acquisition time trigger StopAcquisitionSafely()
    // method, we are not only closing the ROOT file but also
    // disabling acquisition so the widgets need to be fully
    // reset. (The DGScopeDataStorageCloseFile_TB click above allows
    // for continuing acquisition and new files.).
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);
  }
}


// Method to generate a standard detector-esque artificial waveoforms
// to be used in debugging mode when waveforms from the DAQ are not
// available. Method receives the record length of the acquisition
// window and a reference to the being-processed data channel in order
// to fill the channel data with the artificial waveform data. The
// artificial waveform has a quick rise time and longer decay tail,
// which are randomly varied to mimick data acquisition.
void ADAQAcquisition::GenerateArtificialWaveform(int RecordLength, vector<int> &Voltage, 
						 double *Voltage_graph, double VerticalPosition)
{
  // Exponential time constants with small random variations
  const double t1 = 20. - (RNG->Rndm()*10);
  const double t2 = 80. - (RNG->Rndm()*40);;
  
  // The waveform amplitude with small random variations
  const double a = RNG->Rndm() * 30;
  
  // Set an artificial baseline for the pulse
  const double b = 3200;
  
  // Set an artifical polarity for the pulse
  const double p = -1.0;
  
  // Set the number of leading zeros before the waveform begins with
  // small random variations
  const int NumLeadingSamples = 100;
  
  // Fill the Voltage vector with the artificial waveform
  for(int sample=0; sample<RecordLength; sample++){
    
    if(sample < NumLeadingSamples){
      Voltage[sample] = b + VerticalPosition;
      Voltage_graph[sample] = b + VerticalPosition;
    }
    else{
      double t = (sample - NumLeadingSamples)*1.0;
      Voltage[sample] = (p * (a * (t1-t2) * (exp(-t/t1)-exp(-t/t2)))) + b + VerticalPosition;
      Voltage_graph[sample] = Voltage[sample];
    }
  }
}


// The mandatory C++ main function
int main(int argc, char **argv)
{
  // Create a standalone application that runs outside of a ROOT seesion
  TApplication *TheApplication = new TApplication("ADAQAcquisition", &argc, argv);
  
  // Create variables for width and height of the top-level GUI window
  int Width = 1125;
  int Height = 840;

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
  
  // Create an object of type ADAQAcquisition and connect its "CloseWindow" function
  ADAQAcquisition *MainFrame = new ADAQAcquisition(Width, Height);
  MainFrame->Connect("CloseWindow()", "ADAQAcquisition", MainFrame, "HandleDisconnectAndTerminate(bool)");
  
  // Run the standalone application
  TheApplication->Run();
    
  // Clean up memory upon completion
  delete MainFrame;
  
  return 0;
}
