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

#include "AAInterface.hh"
#include "AATypes.hh"
#include "AAVersion.hh"

#include "AAChannelSlots.hh"
#include "AADisplaySlots.hh"
#include "AASubtabSlots.hh"
#include "AATabSlots.hh"
#include "AASettings.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "AAGraphics.hh"


AAInterface::AAInterface()
  : TGMainFrame(gClient->GetRoot()),
    DisplayWidth(1124), DisplayHeight(833), 
    NumDataChannels(8), ColorManager(new TColor), NumVMEBoards(3)
{
  // Allow environmental variable to control small version of GUI
  if(getenv("ADAQACQUISITION_SMALL")!=NULL){
    DisplayWidth = 980;
    DisplayHeight = 650;
  }

  // Create the slot handler classes to receive signals from the
  // widgets; handlers are roughly grouped by GUI tab layout
  ChannelSlots = new AAChannelSlots(this);
  DisplaySlots = new AADisplaySlots(this);
  SubtabSlots = new AASubtabSlots(this);
  TabSlots = new AATabSlots(this);

  TheVMEManager = AAVMEManager::GetInstance();
  
  // A structure to important widget settings;
  TheSettings = new AASettings(NumDataChannels);
  
  /////////////////////////////
  // Initialize HV variables //
  /////////////////////////////
  // Initialize C++ stdlib vectors and maps but using the
  // Boost::Assign functionality for its utility and concision

  // std::vector for HV channel labels
  HVChLabels += 
    "Channel 0 (-)", "Channel 1 (-)", "Channel 2 (-)", 
    "Channel 3 (+)", "Channel 4 (+)", "Channel 5 (+)";
  
  // std::vector to return the ROOT channel power widget ID from the HV channel number
  HVChPower_TB_ID_Vec += 
    (int)HVCh0Power_TB_ID, (int)HVCh1Power_TB_ID, (int)HVCh2Power_TB_ID,
    (int)HVCh3Power_TB_ID, (int)HVCh4Power_TB_ID, (int)HVCh5Power_TB_ID;
  
  // std::map to return the HV channel number from the ROOT channel power widget ID
  insert(HVChPower_TB_ID_Map) 
    ((int)HVCh0Power_TB_ID,0) ((int)HVCh1Power_TB_ID,1) ((int)HVCh2Power_TB_ID,2)
    ((int)HVCh3Power_TB_ID,3) ((int)HVCh4Power_TB_ID,4) ((int)HVCh5Power_TB_ID,5);

  /////////////////////////////
  // Initialize DG variables //
  /////////////////////////////

  DGChannelLabels += 
    "Channel 0", "Channel 1", "Channel 2", "Channel 3", 
    "Channel 4", "Channel 5", "Channel 6", "Channel 7";

  DGChEnable_CB_ID_Vec += 
    (int)DGCh0Enable_CB_ID, (int)DGCh1Enable_CB_ID, (int)DGCh2Enable_CB_ID, 
    (int)DGCh3Enable_CB_ID, (int)DGCh4Enable_CB_ID, (int)DGCh5Enable_CB_ID, 
    (int)DGCh6Enable_CB_ID, (int)DGCh7Enable_CB_ID;
  
  DGChDCOffset_NEL_ID_Vec += 
    (int)DGCh0DCOffset_NEL_ID, (int)DGCh1DCOffset_NEL_ID, (int)DGCh2DCOffset_NEL_ID, 
    (int)DGCh3DCOffset_NEL_ID, (int)DGCh4DCOffset_NEL_ID, (int)DGCh5DCOffset_NEL_ID, 
    (int)DGCh6DCOffset_NEL_ID, (int)DGCh7DCOffset_NEL_ID;

  DGChTriggerThreshold_NEL_ID_Vec += 
    (int)DGCh0TriggerThreshold_NEL_ID, (int)DGCh1TriggerThreshold_NEL_ID, (int)DGCh2TriggerThreshold_NEL_ID, 
    (int)DGCh3TriggerThreshold_NEL_ID, (int)DGCh4TriggerThreshold_NEL_ID, (int)DGCh5TriggerThreshold_NEL_ID, 
    (int)DGCh6TriggerThreshold_NEL_ID, (int)DGCh7TriggerThreshold_NEL_ID;

  DGChBaselineCalcMin_NEL_ID_Vec += 
    (int)DGCh0BaselineCalcMin_NEL_ID, (int)DGCh1BaselineCalcMin_NEL_ID, (int)DGCh2BaselineCalcMin_NEL_ID, 
    (int)DGCh3BaselineCalcMin_NEL_ID, (int)DGCh4BaselineCalcMin_NEL_ID, (int)DGCh5BaselineCalcMin_NEL_ID,
    (int)DGCh6BaselineCalcMin_NEL_ID, (int)DGCh7BaselineCalcMin_NEL_ID;

  DGChBaselineCalcMax_NEL_ID_Vec += 
    (int)DGCh0BaselineCalcMax_NEL_ID, (int)DGCh1BaselineCalcMax_NEL_ID, (int)DGCh2BaselineCalcMax_NEL_ID,
    (int)DGCh3BaselineCalcMax_NEL_ID, (int)DGCh4BaselineCalcMax_NEL_ID, (int)DGCh5BaselineCalcMax_NEL_ID, 
    (int)DGCh6BaselineCalcMax_NEL_ID, (int)DGCh7BaselineCalcMax_NEL_ID;
  
  insert(DGChTriggerThreshold_NEL_ID_Map)
    ((int)DGCh0TriggerThreshold_NEL_ID,0) ((int)DGCh1TriggerThreshold_NEL_ID,1) ((int)DGCh2TriggerThreshold_NEL_ID,2) 
    ((int)DGCh3TriggerThreshold_NEL_ID,3) ((int)DGCh4TriggerThreshold_NEL_ID,4) ((int)DGCh5TriggerThreshold_NEL_ID,5)
    ((int)DGCh6TriggerThreshold_NEL_ID,6) ((int)DGCh7TriggerThreshold_NEL_ID,7);


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
  FillAcquisitionFrame();


  ///////////////////////////////////////////
  // Set manager pointers for later access //
  ///////////////////////////////////////////

  TheVMEManager->SetSettingsPointer(TheSettings);
  
  AAAcquisitionManager::GetInstance()->SetInterfacePointer(this);
  AAAcquisitionManager::GetInstance()->SetSettingsPointer(TheSettings);
  
  AAGraphics::GetInstance()->SetCanvasPointer(DisplayCanvas_EC->GetCanvas());
  AAGraphics::GetInstance()->SetSettingsPointer(TheSettings);


  ////////////////////////////////
  // Final setup of main window //
  ////////////////////////////////

  // Set cleanup upon destruction
  SetCleanup(kDeepCleanup);

  // Connect main window buttons to close properly
  Connect("CloseWindow()", "AAInterface", this, "HandleDisconnectAndTerminate()");

  string TitleString;
  if(VersionString == "Development")
    TitleString = "AIMS Data Acquisition (Development version)               Fear is the mind-killer.";
  else
    TitleString = "AIMS Data Acquisition (Production version " + VersionString + ")               Fear is the mind-killer.";

  SetWindowName(TitleString.c_str());
  MapSubwindows();
  Resize(DisplayWidth, DisplayHeight);
  MapWindow();
}


AAInterface::~AAInterface()
{
  delete TheSettings;
  delete TabSlots;
  delete SubtabSlots;
  delete DisplaySlots;
  delete ChannelSlots;
}


void AAInterface::CreateTopLevelFrames()
{
  TGCanvas *TopFrame_C = new TGCanvas(this, DisplayWidth, DisplayHeight, kRaisedFrame);
  AddFrame(TopFrame_C, new TGLayoutHints(kLHintsCenterX, 40,0,0,0));

  TopFrame = new TGVerticalFrame(TopFrame_C->GetViewPort(), DisplayWidth, DisplayHeight);
  TopFrame->SetBackgroundColor(ColorManager->Number2Pixel(22));
  TopFrame_C->SetContainer(TopFrame);


  ///////////////
  // Tab frame //
  ///////////////
  
  TGHorizontalFrame *TabFrame = new TGHorizontalFrame(TopFrame);
  TabFrame->SetBackgroundColor(ColorManager->Number2Pixel(22));
  
  TopLevelTabs = new TGTab(TabFrame);
  
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

  AcquisitionTab = TopLevelTabs->AddTab(" Acquisition ");
  AcquisitionTab->SetBackgroundColor(ColorManager->Number2Pixel(22));
  AcquisitionFrame = new TGCompositeFrame(AcquisitionTab, 60, 20, kHorizontalFrame);
  AcquisitionFrame->SetBackgroundColor(ColorManager->Number2Pixel(22));
  AcquisitionTab->AddFrame(AcquisitionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  TabFrame->AddFrame(TopLevelTabs, new TGLayoutHints(kLHintsTop, 5,5,5,5));


  ////////////////////////////////////////////
  // Add top level frames to the main frame //
  ////////////////////////////////////////////
  
  TopFrame->AddFrame(TabFrame, new TGLayoutHints(kLHintsCenterX, 35,5,5,5));

  AddFrame(TopFrame, new TGLayoutHints(kLHintsCenterX, 0,0,0,0));
  SetBackgroundColor(ColorManager->Number2Pixel(22));
}


void AAInterface::FillConnectionFrame()
{
  // The main VME connection button
  
  TGGroupFrame *Connection_GF = new TGGroupFrame(ConnectionFrame,"Initiate VME Connection", kVerticalFrame);
  ConnectionFrame->AddFrame(Connection_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));
  Connection_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  Connection_GF->AddFrame(VMEConnect_TB = new TGTextButton(Connection_GF, "Disconnected: click to connect", VMEConnect_TB_ID),
			    new TGLayoutHints(kLHintsExpandX, 5,5,25,5));
  VMEConnect_TB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleConnectionTextButtons()");
  VMEConnect_TB->Resize(500,40);
  VMEConnect_TB->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  VMEConnect_TB->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
  VMEConnect_TB->ChangeOptions(VMEConnect_TB->GetOptions() | kFixedSize);

  // A text view window to capture/display std::cout information

  Connection_GF->AddFrame(ConnectionOutput_TV = new TGTextView(Connection_GF, 700, 400, -42),
			  new TGLayoutHints(kLHintsTop | kLHintsExpandX, 15,15,5,25));
  ConnectionOutput_TV->SetBackground(ColorManager->Number2Pixel(18));
  
  // The VME addresses, address display, and enable/disable widgets

  TGGroupFrame *ModuleSettings_GF = new TGGroupFrame(ConnectionFrame, "VME Module Settings", kHorizontalFrame);
  ConnectionFrame->AddFrame(ModuleSettings_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));
  ModuleSettings_GF->SetTitlePos(TGGroupFrame::kCenter);

  vector<string> AddressTitle;
  AddressTitle += "V1718 base address", "Digitizer base address", "High voltage base address";

  vector<int> BoardEnableID, BoardAddressID, BoardAddress;
  BoardEnableID += (int)V1718BoardEnable_TB_ID, (int)DGBoardEnable_TB_ID, (int)HVBoardEnable_TB_ID;
  BoardAddressID += (int)0, (int)DGBoardAddress_ID, (int)DGBoardAddress_ID;

  BoardAddress += (int)0, (int)TheVMEManager->GetDGAddress(), (int)TheVMEManager->GetHVAddress();

  for(int board=0; board<NumVMEBoards; board++){
    
    TGVerticalFrame *BoardAddress_VF = new TGVerticalFrame(ModuleSettings_GF);
    BoardAddress_VF->AddFrame(new TGLabel(BoardAddress_VF, AddressTitle[board].c_str()), 
			      new TGLayoutHints(kLHintsCenterX, 5,5,5,0));

    TGHorizontalFrame *BoardAddress_HF = new TGHorizontalFrame(BoardAddress_VF);
    BoardAddress_HF->AddFrame(new TGLabel(BoardAddress_HF,"0x"), 
			      new TGLayoutHints(kLHintsExpandY, 5,0,0,5));

    // The V1718 USB-VME board differs from the others in that, as the
    // VME controller, it does not an explictly settable VME
    // address. Its VME address is set automatically when a link is
    // established (see the ADAQBridge class of the ADAQ library for
    // details) Thus, there is special handling of the V1718 controls.
    if(board == 0){
      // Push back a zero to maintain an array size of 3
      BoardAddress_NEF.push_back(0);

      // Create a placeholder alerting the user to the automatically
      // set V1718 VME address
      TGTextEntry *V1718_TE = new TGTextEntry(BoardAddress_HF, "Auto. Set!", 0);
      V1718_TE->SetAlignment(kTextCenterX);
      V1718_TE->Resize(80,20);
      BoardAddress_HF->AddFrame(V1718_TE, new TGLayoutHints(kLHintsExpandY, 5, 5, 0, 5));
    }
    else{
      BoardAddress_NEF.push_back(new TGNumberEntryField(BoardAddress_HF, BoardAddressID[board], 0,
							TGNumberFormat::kNESHex, 
							TGNumberFormat::kNEAPositive));
      BoardAddress_NEF[board]->SetHexNumber(BoardAddress[board]);
      BoardAddress_NEF[board]->Resize(80,20);
      BoardAddress_HF->AddFrame(BoardAddress_NEF[board], new TGLayoutHints(kLHintsExpandY, 5, 5, 0, 5));
    }

    BoardAddress_VF->AddFrame(BoardAddress_HF,new TGLayoutHints(kLHintsExpandY, 5,5,5,5));  

    BoardEnable_TB.push_back(new TGTextButton(BoardAddress_VF, "Board enabled", BoardEnableID[board]));
    BoardEnable_TB[board]->Connect("Clicked()", "AATabSlots", TabSlots, "HandleConnectionTextButtons()");
    BoardEnable_TB[board]->Resize(110,25);
    BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(kGreen+2));
    BoardEnable_TB[board]->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
    BoardEnable_TB[board]->ChangeOptions(BoardEnable_TB[board]->GetOptions() | kFixedSize);
    BoardAddress_VF->AddFrame(BoardEnable_TB[board], new TGLayoutHints(kLHintsCenterX));

    int borderLeft = 30;
    int borderRight = 30;
    if(board == 1){
      borderLeft = 30;
      borderRight = 30;
    }
    ModuleSettings_GF->AddFrame(BoardAddress_VF, 
				new TGLayoutHints(kLHintsTop | kLHintsCenterX, borderLeft,borderRight,5,5));
  }

  if(!TheVMEManager->GetBREnable()){
    BoardEnable_TB[V1718]->SetText("Board disabled");
    BoardEnable_TB[V1718]->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  }

  if(!TheVMEManager->GetDGEnable()){
    BoardEnable_TB[V1720]->SetText("Board disabled");
    BoardEnable_TB[V1720]->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  }

  if(!TheVMEManager->GetHVEnable()){
    BoardEnable_TB[V6534]->SetText("Board disabled");
    BoardEnable_TB[V6534]->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  }
}


void AAInterface::FillRegisterFrame()
{
  const int NumVMEBoards = 3;

  string FrameTitle[NumVMEBoards] = {"V1718 VME/USB Module", "CAEN Digitizer Module", "CAEN High Voltage Module"};

  int ReadAddressID[NumVMEBoards] = {V1718ReadAddress_ID, DGReadAddress_ID, HVReadAddress_ID};
  int ReadValueID[NumVMEBoards] = {V1718ReadValue_ID, DGReadValue_ID, HVReadValue_ID};

  int WriteAddressID[NumVMEBoards] = {V1718WriteAddress_ID, DGWriteAddress_ID, HVWriteAddress_ID};
  int WriteValueID[NumVMEBoards] = {V1718WriteValue_ID, DGWriteValue_ID, HVWriteValue_ID};

  int ReadID[NumVMEBoards] = {V1718Read_ID, DGRead_ID, HVRead_ID};
  int WriteID[NumVMEBoards] = {V1718Write_ID, DGWrite_ID, HVWrite_ID};

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
    Read_TB[board]->Connect("Clicked()", "AATabSlots", TabSlots, "HandleRegisterTextButtons()");
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
    Write_TB[board]->Connect("Clicked()", "AATabSlots", TabSlots, "HandleRegisterTextButtons()");
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


void AAInterface::FillPulserFrame()
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
    V1718PulserStartStop_TB[pulser]->Connect("Pressed()", "AATabSlots", TabSlots, "HandlePulserTextButtons()");
  }  

}


// The "VoltageFrame" holds ROOT widgets for complete control of the
// HV supply board, including real-time monitoring of each channel's
// active voltage and drawn current. Setting the voltage and current
// for an individual channel are disabled while the channel power is
// on. This may be updated in the future to enable real-time changes.
void AAInterface::FillVoltageFrame()
{
  TGVerticalFrame *HVChannelControls_VF = new TGVerticalFrame(VoltageFrame);
  
  const int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
  
  for(int ch=0; ch<HVChannels; ch++){
    
    TGGroupFrame *HVChannel_GF = new TGGroupFrame(HVChannelControls_VF, HVChLabels[ch].c_str(), kHorizontalFrame);
    HVChannel_GF->SetTitlePos(TGGroupFrame::kCenter);
    HVChannelControls_VF->AddFrame(HVChannel_GF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    
    TGVerticalFrame *HVChannelSet_VF = new TGVerticalFrame(HVChannel_GF);
    HVChannel_GF->AddFrame(HVChannelSet_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 15,15,5,0));
    
    // ADAQ number entry for setting the channel voltage
    HVChannelSet_VF->AddFrame(HVChVoltage_NEL[ch] = new ADAQNumberEntryWithLabel(HVChannelSet_VF, "Set Voltage [V]",-1),
			      new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    HVChVoltage_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    HVChVoltage_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    HVChVoltage_NEL[ch]->GetEntry()->SetLimitValues(0, TheVMEManager->GetHVManager()->GetMaxVoltage());
    HVChVoltage_NEL[ch]->GetEntry()->SetNumber(0);
    
    // ADAQ number entry for setting maximum channel current that can be drawn
    HVChannelSet_VF->AddFrame(HVChCurrent_NEL[ch] = new ADAQNumberEntryWithLabel(HVChannelSet_VF, "Set Current [uA]",-1),
			      new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    HVChCurrent_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    HVChCurrent_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    HVChCurrent_NEL[ch]->GetEntry()->SetLimitValues(0, TheVMEManager->GetHVManager()->GetMaxCurrent());
    HVChCurrent_NEL[ch]->GetEntry()->SetNumber(0);

    TGVerticalFrame *HVChannelGet_VF = new TGVerticalFrame(HVChannel_GF);
    HVChannel_GF->AddFrame(HVChannelGet_VF, new TGLayoutHints(kLHintsTop | kLHintsLeft, 15,15,5,0));

    // ADAQ number entry field for displaying the real-time channel voltage [in volts]
    HVChannelGet_VF->AddFrame(HVChVoltageMonitor_NEFL[ch] = new ADAQNumberEntryFieldWithLabel(HVChannelGet_VF, "Active Voltage [V]", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(false);
    
    // ADAQ number entry field for display the real-time channel current [in microamps]
    HVChannelGet_VF->AddFrame(HVChCurrentMonitor_NEFL[ch] = new ADAQNumberEntryFieldWithLabel(HVChannelGet_VF, "Active Current [uA]", -1),
			      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(false);

    // ROOT text button to control channel power. The widget ID is set
    // using a std::vector to assign a unique ID from the ADAQ widget
    // ID enumerator (see file "ADAQEnumerators.hh") for each channel
    HVChannel_GF->AddFrame(HVChPower_TB[ch] = new TGTextButton(HVChannel_GF, "OFF", HVChPower_TB_ID_Vec[ch]),
			   new TGLayoutHints(kLHintsNormal,15,15,15,5));
    HVChPower_TB[ch]->Connect("Pressed()", "AATabSlots", TabSlots, "HandleVoltageTextButtons()");
    HVChPower_TB[ch]->SetToolTipText("Engage high voltage!");
    HVChPower_TB[ch]->Resize(110,50);
    HVChPower_TB[ch]->ChangeOptions(HVChPower_TB[ch]->GetOptions() | kFixedSize);
    HVChPower_TB[ch]->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
    HVChPower_TB[ch]->SetForegroundColor(ColorManager->Number2Pixel(1));
    
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
  HVMonitorEnable_CB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleVoltageTextButtons()");
  HVMonitorEnable_CB->SetState(kButtonUp);
  
  VoltageFrame->AddFrame(HVChannelControls_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
}

 
void AAInterface::FillAcquisitionFrame()
{
  //////////////////////////////
  // Fill left vertical panel //
  //////////////////////////////
  // The left-most vertical subframe in the acquisition frame contains
  // eight digitizer (DG) channel-specific settings. The subframe
  // makes use of the TGCanvas class to incorporate sliders that can
  // be used to view all channel widgets in a smalle frame. Pro'n'shit
  // if I do say so myself. And I do.

  TGCanvas *DGChannelControls_C = new TGCanvas(AcquisitionFrame,300,100,kRaisedFrame);
  AcquisitionFrame->AddFrame(DGChannelControls_C, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  
  TGVerticalFrame *DGChannelControls_VF = new TGVerticalFrame(DGChannelControls_C->GetViewPort(),10,10);
  DGChannelControls_C->SetContainer(DGChannelControls_VF);

  // The widgets and layouts for control of channel-relevant
  // parameters for each of the 8 digitizers is identical although the
  // underlying functionality must correctly identify and set values
  // for each channel correctly. The widgets and layouts are therefore
  // constructed in a "for" loop previously initialized using
  // vectors to assign parameters that are unique to each channel
  // (names and IDs).
  for(int ch=0; ch<NumDataChannels; ch++){

    // Each channel's widgets are grouped under its own TGGroupFrame.
    TGGroupFrame *DGChannelControl_GF = new TGGroupFrame(DGChannelControls_VF, DGChannelLabels[ch].c_str(), kVerticalFrame);
    DGChannelControls_VF->AddFrame(DGChannelControl_GF, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 0));
    DGChannelControl_GF->SetTitlePos(TGGroupFrame::kLeft);
    
    // Horizontal frame to hold the "enable" and "pulse polarity" buttons
    TGHorizontalFrame *DGChannelControl_HF = new TGHorizontalFrame(DGChannelControl_GF);
    DGChannelControl_GF->AddFrame(DGChannelControl_HF);
    
    // ROOT check button to enable channel for digitization
    DGChannelControl_HF->AddFrame(DGChEnable_CB[ch] = new TGCheckButton(DGChannelControl_HF, "Enable", DGChEnable_CB_ID_Vec[ch]),
				  new TGLayoutHints(kLHintsCenterY,0,0,0,0));
    if(ch == 0) 
      DGChEnable_CB[ch]->SetState(kButtonDown);

    // TGLabel for the pulse polarity radio buttons
    DGChannelControl_HF->AddFrame(new TGLabel(DGChannelControl_HF,"Pulse \n polarity"),
				       new TGLayoutHints(kLHintsCenterY,25,0,5,0));

    TGHButtonGroup *DGChPolarity_BG = new TGHButtonGroup(DGChannelControl_HF, "");
    DGChPolarity_BG->SetTitlePos(TGButtonGroup::kCenter);
    DGChPolarity_BG->SetBorderDrawn(false);
    DGChannelControl_HF->AddFrame(DGChPolarity_BG, new TGLayoutHints(kLHintsRight,-1,-15,-10,-10));

    DGChPosPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "+  ", -1);
    DGChNegPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "-", -1);
    DGChNegPolarity_RB[ch]->SetState(kButtonDown);
    DGChPolarity_BG->Show();


    // ADAQ number entry to set channel's vertical position on graph [ADC]
    DGChannelControl_GF->AddFrame(DGChVerticalPosition_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Vert. Position (ADC)", -1),
				  new TGLayoutHints(kLHintsNormal));
    DGChVerticalPosition_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChVerticalPosition_NEL[ch]->GetEntry()->SetNumber(0);
    DGChVerticalPosition_NEL[ch]->GetEntry()->Resize(55,20);

    // ADAQ number entry to set channel's DAC offset 
    DGChannelControl_GF->AddFrame(DGChDCOffset_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "DC offset (hex)", DGChDCOffset_NEL_ID_Vec[ch]),
				  new TGLayoutHints(kLHintsNormal));
    DGChDCOffset_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESHex);
    DGChDCOffset_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    DGChDCOffset_NEL[ch]->GetEntry()->SetLimitValues(0x0000,0xffff);
    DGChDCOffset_NEL[ch]->GetEntry()->SetNumber(0x8000);
    DGChDCOffset_NEL[ch]->GetEntry()->Resize(55,20);
    
    // ADAQ number entry to set channel's trigger threshold [ADC]
    DGChannelControl_GF->AddFrame(DGChTriggerThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Trig. Threshold (ADC)", DGChTriggerThreshold_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
    DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(2500);
    DGChTriggerThreshold_NEL[ch]->GetEntry()->Resize(55,20);
    
    // ADAQ number entry to set minimum sample for baseline calculation [sample]
    DGChannelControl_GF->AddFrame(DGChBaselineCalcMin_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Baseline min. (sample)", DGChBaselineCalcMin_NEL_ID_Vec[ch]),
				  new TGLayoutHints(kLHintsNormal));
    DGChBaselineCalcMin_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
    
    DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetNumber(10);
    DGChBaselineCalcMin_NEL[ch]->GetEntry()->Resize(55,20);

    // ADAQ number entry to set maximum sample for baseline calculation [sample]
    DGChannelControl_GF->AddFrame(DGChBaselineCalcMax_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Baseline max. (sample)", DGChBaselineCalcMax_NEL_ID_Vec[ch]),
				  new TGLayoutHints(kLHintsNormal));
    DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetNumber(45);
    DGChBaselineCalcMax_NEL[ch]->GetEntry()->Resize(55,20);

    DGChannelControl_GF->AddFrame(DGChZSThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "ZS threshold (ADC)", -1),
				       new TGLayoutHints(kLHintsNormal));
    DGChZSThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChZSThreshold_NEL[ch]->GetEntry()->SetNumber(0);
    DGChZSThreshold_NEL[ch]->GetEntry()->Resize(55,20);


    TGHorizontalFrame *ZS_HF0 = new TGHorizontalFrame(DGChannelControl_GF);
    DGChannelControl_GF->AddFrame(ZS_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

    ZS_HF0->AddFrame(DGChZSForward_NEL[ch] = new ADAQNumberEntryWithLabel(ZS_HF0, "ZS Frwd", -1),
		     new TGLayoutHints(kLHintsNormal, 0,0,0,0));
    DGChZSForward_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChZSForward_NEL[ch]->GetEntry()->SetNumber(0);
    DGChZSForward_NEL[ch]->GetEntry()->Resize(55,20);

    ZS_HF0->AddFrame(DGChZSBackward_NEL[ch] = new ADAQNumberEntryWithLabel(ZS_HF0, "ZS Back", -1),
		     new TGLayoutHints(kLHintsNormal, 10,0,0,0));
    DGChZSBackward_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGChZSBackward_NEL[ch]->GetEntry()->SetNumber(0);
    DGChZSBackward_NEL[ch]->GetEntry()->Resize(55,20);

    TGHorizontalFrame *ZS_HF1 = new TGHorizontalFrame(DGChannelControl_GF);
    DGChannelControl_GF->AddFrame(ZS_HF1, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

    ZS_HF1->AddFrame(new TGLabel(ZS_HF1, "ZS Logic"),
		     new TGLayoutHints(kLHintsNormal, 60,0,5,0));
    
    TGHButtonGroup *ZSLogicButtons_BG = new TGHButtonGroup(ZS_HF1,"");
    ZSLogicButtons_BG->SetBorderDrawn(false);
    ZS_HF1->AddFrame(ZSLogicButtons_BG, new TGLayoutHints(kLHintsNormal, -1,-15,-10,-10));

    DGChZSPosLogic_RB[ch] = new TGRadioButton(ZSLogicButtons_BG, "+  ", -1);
    DGChZSPosLogic_RB[ch]->SetState(kButtonDown);
    
    DGChZSNegLogic_RB[ch] = new TGRadioButton(ZSLogicButtons_BG, "-", -1);
  }
  

  ////////////////////////////////
  // Fill DG display frame //
  ////////////////////////////////
  // The display frame contains the ROOT TRootEmbeddedCanvas, which
  // displays the waveforms and pulse heigh spectra, and two sliders,
  // which control the vertical and horizontal zoom of the graphs
  // along with spectrum calibration via the 3rd slider on the
  // horizontal axis.

  TGVerticalFrame *DGDisplayAndControls_VF = new TGVerticalFrame(AcquisitionFrame);
  
  TGVerticalFrame *Display_VF = new TGVerticalFrame(DGDisplayAndControls_VF);
  Display_VF->SetBackgroundColor(ColorManager->Number2Pixel(22));
  
  TGHorizontalFrame *DGScopeDisplayAndSlider_HF = new TGHorizontalFrame(Display_VF);
  DGScopeDisplayAndSlider_HF->SetBackgroundColor(ColorManager->Number2Pixel(22));
  Display_VF->AddFrame(DGScopeDisplayAndSlider_HF, new TGLayoutHints(kLHintsNormal,0,0,5,0));

  // ROOT double slider for control of the min/max of vertical axis, ie, zoom
  DGScopeDisplayAndSlider_HF->AddFrame(DisplayVerticalScale_DVS = new TGDoubleVSlider(DGScopeDisplayAndSlider_HF, 430, kDoubleScaleBoth, DisplayVerticalScale_DVS_ID, kVerticalFrame, ColorManager->Number2Pixel(17),true,false),
				       new TGLayoutHints(kLHintsNormal, 0, 0, 0, 0));
  DisplayVerticalScale_DVS->SetRange(0,1);
  DisplayVerticalScale_DVS->SetPosition(0,1);
  DisplayVerticalScale_DVS->SetBackgroundColor(ColorManager->Number2Pixel(18));
  DisplayVerticalScale_DVS->Connect("PositionChanged()", "AADisplaySlots", DisplaySlots, "HandleDoubleSliders()");
  
  // ROOT embdedded canvas for display of waveforms and spectra
  DGScopeDisplayAndSlider_HF->AddFrame(DisplayCanvas_EC = new TRootEmbeddedCanvas("DisplayCanvas_EC", DGScopeDisplayAndSlider_HF, 670, 430),
				       new TGLayoutHints(kLHintsCenterX, 0,0,0,0));
  DisplayCanvas_EC->GetCanvas()->SetFillColor(0);
  DisplayCanvas_EC->GetCanvas()->SetFrameFillColor(0);
  DisplayCanvas_EC->GetCanvas()->SetGrid(true, true);
  DisplayCanvas_EC->GetCanvas()->SetBorderMode(0);
  DisplayCanvas_EC->GetCanvas()->SetLeftMargin(0.13);
  DisplayCanvas_EC->GetCanvas()->SetBottomMargin(0.12);
  DisplayCanvas_EC->GetCanvas()->SetTopMargin(0.08);
  DisplayCanvas_EC->GetCanvas()->SetRightMargin(0.05);

  // ROOT triple slider. The "double" slider features are used to //
  // control the min/max of the horizontal axis, ie, zoom; The "third"
  // slider is used for graphical valibration of the pulse height
  // spectrum when DGScope is set to "calibration mode" while
  // acquiring data in "spectrum mode"
  Display_VF->AddFrame(DisplayHorizontalScale_THS = new TGTripleHSlider(Display_VF, 670, kDoubleScaleBoth, DisplayHorizontalScale_THS_ID, kVerticalFrame, ColorManager->Number2Pixel(17)),
		       new TGLayoutHints(kLHintsRight, 5, 0, 0, 5));
  DisplayHorizontalScale_THS->SetRange(0,1);
  DisplayHorizontalScale_THS->SetPosition(0,1);
  DisplayHorizontalScale_THS->SetPointerPosition(0.5);
  DisplayHorizontalScale_THS->SetBackgroundColor(ColorManager->Number2Pixel(18));
  DisplayHorizontalScale_THS->Connect("PositionChanged()", "AADisplaySlots", DisplaySlots, "HandleDoubleSliders()");
  DisplayHorizontalScale_THS->Connect("PointerPositionChanged()", "AADisplaySlots", DisplaySlots, "HandleSliderPointers()");
  

  TGHorizontalFrame *DGScopeDisplayButtons_HF = new TGHorizontalFrame(Display_VF);
  DGScopeDisplayButtons_HF->SetBackgroundColor(ColorManager->Number2Pixel(22));
  Display_VF->AddFrame(DGScopeDisplayButtons_HF, new TGLayoutHints(kLHintsNormal, 0,0,5,0));

  // ROOT text button for starting/stopping data acquisition by the digitizer
  DGScopeDisplayButtons_HF->AddFrame(AQStartStop_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Stopped", AQStartStop_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  AQStartStop_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");
  AQStartStop_TB->Resize(300,30);
  AQStartStop_TB->ChangeOptions(AQStartStop_TB->GetOptions() | kFixedSize);
  AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  AQStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
  

  // ROOT text button for manually triggering of DGScope acquisition
  DGScopeDisplayButtons_HF->AddFrame(AQTrigger_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Manual trigger", AQTrigger_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  AQTrigger_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");
  AQTrigger_TB->Resize(175,30);
  AQTrigger_TB->ChangeOptions(AQTrigger_TB->GetOptions() | kFixedSize);
  
  DGScopeDisplayButtons_HF->AddFrame(DisplayUpdate_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Update display", DisplayUpdate_TB_ID),
				     new TGLayoutHints(kLHintsCenterX, 5,5,0,0));
  DisplayUpdate_TB->Resize(175,30);
  DisplayUpdate_TB->ChangeOptions(DisplayUpdate_TB->GetOptions() | kFixedSize);
  DisplayUpdate_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");

  TGHorizontalFrame *DGScopeDisplayControls_HF = new TGHorizontalFrame(Display_VF);
  Display_VF->AddFrame(DGScopeDisplayControls_HF,
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
 
  TGHorizontalFrame *SubtabFrame = new TGHorizontalFrame(DGDisplayAndControls_VF);

  TGTab *AQControlSubtabs = new TGTab(SubtabFrame);

  TGCompositeFrame *AcquisitionSubtab = AQControlSubtabs->AddTab("Acquisition control");
  TGCompositeFrame *AcquisitionSubframe = new TGCompositeFrame(AcquisitionSubtab, 0, 0, kHorizontalFrame);
  AcquisitionSubtab->AddFrame(AcquisitionSubframe);
  
  TGCompositeFrame *SpectrumSubtab = AQControlSubtabs->AddTab("Spectrum creation");
  TGCompositeFrame *SpectrumSubframe = new TGCompositeFrame(SpectrumSubtab, 0, 0, kHorizontalFrame);
  SpectrumSubtab->AddFrame(SpectrumSubframe);
  
  TGCompositeFrame *GraphicsSubtab = AQControlSubtabs->AddTab("Graphics settings");
  TGCompositeFrame *GraphicsSubframe = new TGCompositeFrame(GraphicsSubtab, 0, 0, kHorizontalFrame);
  GraphicsSubtab->AddFrame(GraphicsSubframe);
  
  TGCompositeFrame *DataSubtab = AQControlSubtabs->AddTab("Peristent storage");
  TGCompositeFrame *DataSubframe = new TGCompositeFrame(DataSubtab, 0, 0, kHorizontalFrame);
  DataSubtab->AddFrame(DataSubframe);

  SubtabFrame->AddFrame(AQControlSubtabs, new TGLayoutHints(kLHintsTop, 0,0,0,0));


  ////////////////////
  // Scope settings //
  ////////////////////

  TGVerticalFrame *DGScopeModeAndTrigger_VF = new TGVerticalFrame(AcquisitionSubframe);
  AcquisitionSubframe->AddFrame(DGScopeModeAndTrigger_VF, new TGLayoutHints(kLHintsNormal,0,0,0,0));

  ////////////////
  // Mode controls 

  // ROOT radio buttons to specify operational mode of DGScope:
  // "Waveform" == oscilloscope, "Spectrum" == MCA, "Blank" == display
  // no graphics for high data throughput
  TGButtonGroup *DGScopeMode_BG = new TGButtonGroup(DGScopeModeAndTrigger_VF, "Scope Display Mode", kVerticalFrame);
  DGScopeMode_BG->SetTitlePos(TGButtonGroup::kCenter);
  DGScopeModeAndTrigger_VF->AddFrame(DGScopeMode_BG, new TGLayoutHints(kLHintsExpandX,5,5,5,5));
  
  AQWaveform_RB = new TGRadioButton(DGScopeMode_BG, "Digitized waveform", AQWaveform_RB_ID);
  AQWaveform_RB->SetState(kButtonDown);

  AQSpectrum_RB = new TGRadioButton(DGScopeMode_BG, "Pulse spectrum", AQSpectrum_RB_ID);

  AQHighRate_RB = new TGRadioButton(DGScopeMode_BG, "High-rate (updateable)", AQHighRate_RB_ID);

  AQUltraRate_RB = new TGRadioButton(DGScopeMode_BG, "Ultra-rate (non-updateable)", AQUltraRate_RB_ID);
  
  DGScopeMode_BG->Show();

  ///////////////////
  // Trigger controls
  
  TGGroupFrame *DGTriggerControls_GF = new TGGroupFrame(DGScopeModeAndTrigger_VF, "Trigger Control", kVerticalFrame);
  DGTriggerControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeModeAndTrigger_VF->AddFrame(DGTriggerControls_GF, new TGLayoutHints(kLHintsCenterX,5,5,0,0));


  // ADAQ combo box to enable specification of trigger type
  DGTriggerControls_GF->AddFrame(DGTriggerType_CBL = new ADAQComboBoxWithLabel(DGTriggerControls_GF, "Type", DGTriggerType_CBL_ID),
				 new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGTriggerType_CBL->GetComboBox()->AddEntry("External (NIM)",0);
  DGTriggerType_CBL->GetComboBox()->AddEntry("External (TTL)",1);
  DGTriggerType_CBL->GetComboBox()->AddEntry("Automatic",2);
  DGTriggerType_CBL->GetComboBox()->AddEntry("Software",3);
  DGTriggerType_CBL->GetComboBox()->Select(2);
  DGTriggerType_CBL->GetComboBox()->Resize(110,20);
  DGTriggerType_CBL->GetComboBox()->ChangeOptions(DGTriggerType_CBL->GetComboBox()->GetOptions() | kFixedSize);

  // ADAQ combo box to enable specification of trigger type
  DGTriggerControls_GF->AddFrame(DGTriggerEdge_CBL = new ADAQComboBoxWithLabel(DGTriggerControls_GF, "Edge", DGTriggerEdge_CBL_ID),
				 new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGTriggerEdge_CBL->GetComboBox()->AddEntry("Rising",0);
  DGTriggerEdge_CBL->GetComboBox()->AddEntry("Falling",1);
  DGTriggerEdge_CBL->GetComboBox()->Select(0);
  DGTriggerEdge_CBL->GetComboBox()->Resize(110,20);
  DGTriggerEdge_CBL->GetComboBox()->ChangeOptions(DGTriggerEdge_CBL->GetComboBox()->GetOptions() | kFixedSize);
  
  DGTriggerControls_GF->AddFrame(DGTriggerCoincidenceEnable_CB = new TGCheckButton(DGTriggerControls_GF, "Coincidence triggering", DGTriggerCoincidenceEnable_CB_ID),
				 new TGLayoutHints(kLHintsNormal,5,5,0,0));
  DGTriggerCoincidenceEnable_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  DGTriggerControls_GF->AddFrame(DGTriggerCoincidenceLevel_CBL = new ADAQComboBoxWithLabel(DGTriggerControls_GF, "Level", DGTriggerCoincidenceLevel_CBL_ID),
				 new TGLayoutHints(kLHintsNormal,5,5,0,0));
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("2 channel",1);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("3 channel",2);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("4 channel",3);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("5 channel",4);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("6 channel",5);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("7 channel",6);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->AddEntry("8 channel",7);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->Select(1);
  DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(false);
  


  ///////////////////////
  // Acquisition controls

  TGGroupFrame *DGScopeAcquisitionControls_GF = new TGGroupFrame(AcquisitionSubframe, "Acquisition", kVerticalFrame);
  DGScopeAcquisitionControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  AcquisitionSubframe->AddFrame(DGScopeAcquisitionControls_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ADAQ number entry specifying number of samples
  DGScopeAcquisitionControls_GF->AddFrame(DGRecordLength_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Record length (#)", DGRecordLength_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DGRecordLength_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGRecordLength_NEL->GetEntry()->SetNumber(2000);

  // ADAQ number entry specifying the percentage of the acquisition
  // window that is behind (or after) the triggern (all channels)
  DGScopeAcquisitionControls_GF->AddFrame(DGPostTrigger_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Post trigger (%)", DGPostTriggerSize_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DGPostTrigger_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGPostTrigger_NEL->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
  DGPostTrigger_NEL->GetEntry()->SetLimitValues(0,100);
  DGPostTrigger_NEL->GetEntry()->SetNumber(50);
  
  DGScopeAcquisitionControls_GF->AddFrame(AQTime_NEL = new ADAQNumberEntryWithLabel(DGScopeAcquisitionControls_GF, "Acquisition time (s)", AQTime_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  AQTime_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  AQTime_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  AQTime_NEL->GetEntry()->SetNumber(10);

  DGScopeAcquisitionControls_GF->AddFrame(AQTimer_NEFL = new ADAQNumberEntryFieldWithLabel(DGScopeAcquisitionControls_GF, "Countdown", AQTimer_NEFL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  AQTimer_NEFL->GetEntry()->SetFormat(TGNumberFormat::kNESMinSec);
  AQTimer_NEFL->GetEntry()->SetNumber(10);
  AQTimer_NEFL->GetEntry()->SetState(false);

  TGHorizontalFrame *DGScopeTimerButtons_HF = new TGHorizontalFrame(DGScopeAcquisitionControls_GF);
  DGScopeAcquisitionControls_GF->AddFrame(DGScopeTimerButtons_HF);
  
  DGScopeTimerButtons_HF->AddFrame(AQTimerStart_TB = new TGTextButton(DGScopeTimerButtons_HF, "Start timer", AQTimerStart_TB_ID),
				   new TGLayoutHints(kLHintsNormal, 5,5,0,0));
  AQTimerStart_TB->Resize(100, 30);
  AQTimerStart_TB->ChangeOptions(AQTimerStart_TB->GetOptions() | kFixedSize);
  AQTimerStart_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
  DGScopeTimerButtons_HF->AddFrame(AQTimerAbort_TB = new TGTextButton(DGScopeTimerButtons_HF, "Abort timer", AQTimerAbort_TB_ID),
				   new TGLayoutHints(kLHintsNormal, 5,5,0,0));
  AQTimerAbort_TB->Resize(100, 30);
  AQTimerAbort_TB->ChangeOptions(AQTimerAbort_TB->GetOptions() | kFixedSize);
  AQTimerAbort_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  

  // V1720 readout controls
  TGGroupFrame *DGScopeReadoutControls_GF = new TGGroupFrame(AcquisitionSubframe, "Readout", kVerticalFrame);
  DGScopeReadoutControls_GF->SetTitlePos(TGGroupFrame::kCenter);
  AcquisitionSubframe->AddFrame(DGScopeReadoutControls_GF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeReadoutControls_GF->AddFrame(DGEventsBeforeReadout_NEL = new ADAQNumberEntryWithLabel(DGScopeReadoutControls_GF, "Events before readout", DGEventsBeforeReadout_NEL_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGEventsBeforeReadout_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  DGEventsBeforeReadout_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGEventsBeforeReadout_NEL->GetEntry()->SetNumber(5);
 
  DGScopeReadoutControls_GF->AddFrame(DGCheckBufferStatus_TB = new TGTextButton(DGScopeReadoutControls_GF, "Check FPGA Buffer", CheckBufferStatus_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGCheckBufferStatus_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGCheckBufferStatus_TB->Resize(150,30);
  DGCheckBufferStatus_TB->ChangeOptions(DGCheckBufferStatus_TB->GetOptions() | kFixedSize);
  
  DGScopeReadoutControls_GF->AddFrame(DGBufferStatus_TE = new TGTextEntry(DGScopeReadoutControls_GF, "<Click above check!>",-1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGBufferStatus_TE->SetAlignment(kTextCenterX);
  DGBufferStatus_TE->Resize(200,30);
  DGBufferStatus_TE->ChangeOptions(DGBufferStatus_TE->GetOptions() | kFixedSize);


  DGScopeReadoutControls_GF->AddFrame(AQDataReductionEnable_CB = new TGCheckButton(DGScopeReadoutControls_GF, "Enable data reduction", AQDataReductionEnable_CB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));

  DGScopeReadoutControls_GF->AddFrame(AQDataReductionFactor_NEL = new ADAQNumberEntryWithLabel(DGScopeReadoutControls_GF, "Data reduction factor", AQDataReductionFactor_NEL_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  AQDataReductionFactor_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  AQDataReductionFactor_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  AQDataReductionFactor_NEL->GetEntry()->SetNumber(1);

  DGScopeReadoutControls_GF->AddFrame(DGZSEnable_CB = new TGCheckButton(DGScopeReadoutControls_GF, "Enable zero-suppression", DGZSEnable_CB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));


  ///////////////////////
  // Spectrum settings //
  ///////////////////////

  ////////////
  // Histogram  

  TGGroupFrame *SpectrumHistogram_GF = new TGGroupFrame(SpectrumSubframe, "Histogram", kVerticalFrame);
  SpectrumHistogram_GF->SetTitlePos(TGGroupFrame::kCenter);
  SpectrumSubframe->AddFrame(SpectrumHistogram_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));

  // ADAQ combo box for selecting the channel for display spectrum
  SpectrumHistogram_GF->AddFrame(SpectrumChannel_CBL = new ADAQComboBoxWithLabel(SpectrumHistogram_GF, "", SpectrumChannel_CBL_ID),
					new TGLayoutHints(kLHintsNormal,0,0,5,5));
  for(uint32_t ch=0; ch<8; ch++)
    SpectrumChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  SpectrumChannel_CBL->GetComboBox()->Select(0);
  SpectrumChannel_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");
  
  // ADAQ number entry to specify number of bins used in the spectra histogram
  SpectrumHistogram_GF->AddFrame(SpectrumNumBins_NEL = new ADAQNumberEntryWithLabel(SpectrumHistogram_GF, "Number of bins  ", SpectrumNumBins_NEL_ID),
					new TGLayoutHints(kLHintsLeft,0,0,5,0));
  SpectrumNumBins_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumNumBins_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  SpectrumNumBins_NEL->GetEntry()->SetNumber(100);

  // ADAQ number entry to specify the maximum bin in the spectra histogram
  SpectrumHistogram_GF->AddFrame(SpectrumMinBin_NEL = new ADAQNumberEntryWithLabel(SpectrumHistogram_GF, "Minimum bin", SpectrumMinBin_NEL_ID),
					new TGLayoutHints(kLHintsLeft,0,0,0,0));
  SpectrumMinBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  SpectrumMinBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumMinBin_NEL->GetEntry()->SetNumber(0.);

  SpectrumHistogram_GF->AddFrame(SpectrumMaxBin_NEL = new ADAQNumberEntryWithLabel(SpectrumHistogram_GF, "Maximum bin", SpectrumMaxBin_NEL_ID),
					new TGLayoutHints(kLHintsLeft,0,0,0,5));
  SpectrumMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumMaxBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumMaxBin_NEL->GetEntry()->SetNumber(30000.);

  TGHorizontalFrame *SpectrumAxis_HF = new TGHorizontalFrame(SpectrumHistogram_GF);
  SpectrumHistogram_GF->AddFrame(SpectrumAxis_HF, new TGLayoutHints(kLHintsNormal,0,0,0,0));


  /////////////////
  // Pulse analysis

  TGGroupFrame *SpectrumAnalysis_GF = new TGGroupFrame(SpectrumSubframe,"Analysis",kVerticalFrame);
  SpectrumAnalysis_GF->SetTitlePos(TGGroupFrame::kCenter);
  SpectrumSubframe->AddFrame(SpectrumAnalysis_GF);

  TGHButtonGroup *SpectrumAnalysis_BG = new TGHButtonGroup(SpectrumAnalysis_GF,"Analysis");
  SpectrumAnalysis_BG->SetBorderDrawn(false);
  SpectrumAnalysis_GF->AddFrame(SpectrumAnalysis_BG, new TGLayoutHints(kLHintsNormal,-13,0,0,0));
  
  SpectrumPulseHeight_RB = new TGRadioButton(SpectrumAnalysis_BG, "PHS  ", SpectrumPulseHeight_RB_ID);
  SpectrumPulseHeight_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  
  SpectrumPulseArea_RB = new TGRadioButton(SpectrumAnalysis_BG, "PAS", SpectrumPulseArea_RB_ID);
  SpectrumPulseArea_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  SpectrumPulseArea_RB->SetState(kButtonDown);

  SpectrumAnalysis_GF->AddFrame(SpectrumLDEnable_CB = new TGCheckButton(SpectrumAnalysis_GF, "LD Enable", SpectrumLDEnable_CB_ID),
				new TGLayoutHints(kLHintsNormal, 0,0,0,5));
  
  SpectrumAnalysis_GF->AddFrame(SpectrumLLD_NEL = new ADAQNumberEntryWithLabel(SpectrumAnalysis_GF, "LLD (ADC/energy)", SpectrumLLD_NEL_ID),
				new TGLayoutHints(kLHintsNormal,0,0,-2,0));
  SpectrumLLD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumLLD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumLLD_NEL->GetEntry()->SetNumber(0);

  SpectrumAnalysis_GF->AddFrame(SpectrumULD_NEL = new ADAQNumberEntryWithLabel(SpectrumAnalysis_GF, "ULD (ADC/energy)", SpectrumULD_NEL_ID),
				new TGLayoutHints(kLHintsNormal,0,0,0,0));
  SpectrumULD_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumULD_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumULD_NEL->GetEntry()->SetNumber(30000);

  SpectrumAnalysis_GF->AddFrame(SpectrumLDTrigger_CB = new TGCheckButton(SpectrumAnalysis_GF,"LD trigger to file", SpectrumLDTrigger_CB_ID),
				new TGLayoutHints(kLHintsNormal,0,0,0,0));
  
  SpectrumAnalysis_GF->AddFrame(SpectrumLDTriggerChannel_CBL = new ADAQComboBoxWithLabel(SpectrumAnalysis_GF, "LD Channel", SpectrumLDTriggerChannel_CBL_ID),
				new TGLayoutHints(kLHintsNormal,0,0,0,5));
  
  for(uint32_t ch=0; ch<8; ch++)
    SpectrumLDTriggerChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  SpectrumLDTriggerChannel_CBL->GetComboBox()->Select(0);

  
  //////////////
  // Calibration
  
  TGGroupFrame *SpectrumCalibration_GF = new TGGroupFrame(SpectrumSubframe, "Energy calibration", kVerticalFrame);
  SpectrumSubframe->AddFrame(SpectrumCalibration_GF, new TGLayoutHints(kLHintsNormal,5,5,0,0));
  
  TGHorizontalFrame *SpectrumCalibration_HF0 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  // Energy calibration 
  SpectrumCalibration_HF0->AddFrame(SpectrumCalibration_CB = new TGCheckButton(SpectrumCalibration_HF0, "Make it so", SpectrumCalibration_CB_ID),
				    new TGLayoutHints(kLHintsLeft, 0,0,5,0));
  SpectrumCalibration_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  SpectrumCalibration_CB->SetState(kButtonUp);

  
  SpectrumCalibration_HF0->AddFrame(SpectrumUseCalibrationSlider_CB = new TGCheckButton(SpectrumCalibration_HF0, "Use slider", SpectrumUseCalibrationSlider_CB_ID),
				    new TGLayoutHints(kLHintsLeft,25,5,5,0));
  SpectrumUseCalibrationSlider_CB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  SpectrumUseCalibrationSlider_CB->SetState(kButtonDown);
  SpectrumUseCalibrationSlider_CB->SetEnabled(false);

  /*
  SpectrumCalibration_HF1->AddFrame(SpectrumCalibrationStandard_RB = new TGRadioButton(SpectrumCalibration_HF1, "Standard", SpectrumCalibrationStandard_RB_ID),
					   new TGLayoutHints(kLHintsNormal, 10,5,5,5));
  SpectrumCalibrationStandard_RB->SetState(kButtonDown);
  SpectrumCalibrationStandard_RB->SetState(kButtonDisabled);
  SpectrumCalibrationStandard_RB->Connect("Clicked()", "AAInterface", this, "HandleRadioButtons()");
  
  SpectrumCalibration_HF1->AddFrame(SpectrumCalibrationEdgeFinder_RB = new TGRadioButton(SpectrumCalibration_HF1, "Edge finder", SpectrumCalibrationEdgeFinder_RB_ID),
				       new TGLayoutHints(kLHintsNormal, 30,5,5,5));
  SpectrumCalibrationEdgeFinder_RB->SetState(kButtonDisabled);
  SpectrumCalibrationEdgeFinder_RB->Connect("Clicked()", "AAInterface", this, "HandleRadioButtons()");


  TGVerticalFrame *SpectrumCalibration_VF = new TGVerticalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_VF);
  */

  TGHorizontalFrame *SpectrumCalibration_HF1 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF1, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  SpectrumCalibration_HF1->AddFrame(SpectrumCalibrationPoint_CBL = new ADAQComboBoxWithLabel(SpectrumCalibration_HF1, "", SpectrumCalibrationPoint_CBL_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,10,3));
  SpectrumCalibrationPoint_CBL->GetComboBox()->Resize(150,20);
  SpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0",0);
  SpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
  SpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(false);
  SpectrumCalibrationPoint_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");
  SpectrumUseCalibrationSlider_CB->SetState(kButtonDisabled);
  
  SpectrumCalibration_GF->AddFrame(SpectrumCalibrationEnergy_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Energy (keV or MeV)", SpectrumCalibrationEnergy_NEL_ID),
				   new TGLayoutHints(kLHintsLeft,0,0,0,0));
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
  SpectrumCalibrationEnergy_NEL->GetEntry()->Connect("ValueSet(long)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");

  SpectrumCalibration_GF->AddFrame(SpectrumCalibrationPulseUnit_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Pulse unit (ADC)", SpectrumCalibrationPulseUnit_NEL_ID),
				       new TGLayoutHints(kLHintsLeft,0,0,0,5));
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->Connect("ValueSet(long)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");

  TGHorizontalFrame *SpectrumCalibration_HF2 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF2);
  
  // Set point text button
  SpectrumCalibration_HF2->AddFrame(SpectrumCalibrationSetPoint_TB = new TGTextButton(SpectrumCalibration_HF2, "Set Pt.", SpectrumCalibrationSetPoint_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SpectrumCalibrationSetPoint_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationSetPoint_TB->Resize(100,25);
  SpectrumCalibrationSetPoint_TB->ChangeOptions(SpectrumCalibrationSetPoint_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);

  // Calibrate text button
  SpectrumCalibration_HF2->AddFrame(SpectrumCalibrationCalibrate_TB = new TGTextButton(SpectrumCalibration_HF2, "Calibrate", SpectrumCalibrationCalibrate_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  SpectrumCalibrationCalibrate_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationCalibrate_TB->Resize(100,25);
  SpectrumCalibrationCalibrate_TB->ChangeOptions(SpectrumCalibrationCalibrate_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);
  
  TGHorizontalFrame *SpectrumCalibration_HF3 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF3);
  
  // Plot text button
  SpectrumCalibration_HF3->AddFrame(SpectrumCalibrationPlot_TB = new TGTextButton(SpectrumCalibration_HF3, "Plot", SpectrumCalibrationPlot_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SpectrumCalibrationPlot_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationPlot_TB->Resize(100,25);
  SpectrumCalibrationPlot_TB->ChangeOptions(SpectrumCalibrationPlot_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationPlot_TB->SetState(kButtonDisabled);

  // Reset text button
  SpectrumCalibration_HF3->AddFrame(SpectrumCalibrationReset_TB = new TGTextButton(SpectrumCalibration_HF3, "Reset", SpectrumCalibrationReset_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  SpectrumCalibrationReset_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationReset_TB->Resize(100,25);
  SpectrumCalibrationReset_TB->ChangeOptions(SpectrumCalibrationReset_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationReset_TB->SetState(kButtonDisabled);


  TGHorizontalFrame *SpectrumCalibration_HF4 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF4);

  // Load from file text button
  SpectrumCalibration_HF4->AddFrame(SpectrumCalibrationLoad_TB = new TGTextButton(SpectrumCalibration_HF4, "Load", SpectrumCalibrationLoad_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SpectrumCalibrationLoad_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationLoad_TB->Resize(100,25);
  SpectrumCalibrationLoad_TB->ChangeOptions(SpectrumCalibrationLoad_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationLoad_TB->SetState(kButtonDisabled);

  // Write to file text button
  SpectrumCalibration_HF4->AddFrame(SpectrumCalibrationWrite_TB = new TGTextButton(SpectrumCalibration_HF4, "Write", SpectrumCalibrationWrite_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  SpectrumCalibrationWrite_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  SpectrumCalibrationWrite_TB->Resize(100,25);
  SpectrumCalibrationWrite_TB->ChangeOptions(SpectrumCalibrationLoad_TB->GetOptions() | kFixedSize);
  SpectrumCalibrationWrite_TB->SetState(kButtonDisabled);


  //////////////////////
  // Display settings //
  //////////////////////

  ////////////////////////////
  // Title names and positions 

  TGGroupFrame *DisplayTitle_GF = new TGGroupFrame(GraphicsSubframe, "Titles", kVerticalFrame);
  DisplayTitle_GF->SetTitlePos(TGGroupFrame::kCenter);
  GraphicsSubframe->AddFrame(DisplayTitle_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  DisplayTitle_GF->AddFrame(DisplayTitlesEnable_CB = new TGCheckButton(DisplayTitle_GF, "Override defaults", DisplayTitlesEnable_CB_ID),
			    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DisplayTitlesEnable_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  DisplayTitle_GF->AddFrame(DisplayTitle_TEL = new ADAQTextEntryWithLabel(DisplayTitle_GF, "Plot", DisplayTitle_TEL_ID),
			    new TGLayoutHints(kLHintsNormal,5,5,5,5));
  
  // ADAQ text entries and number entries for specifying the DGScope title, axes title, and axes position
  
  DisplayTitle_GF->AddFrame(DisplayXTitle_TEL = new ADAQTextEntryWithLabel(DisplayTitle_GF, "X-axis", DisplayXTitle_TEL_ID),
			    new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DisplayXTitle_TEL->GetEntry()->SetText("");

  TGHorizontalFrame *XSizeAndOffset_HF = new TGHorizontalFrame(DisplayTitle_GF);
  DisplayTitle_GF->AddFrame(XSizeAndOffset_HF, new TGLayoutHints(kLHintsNormal, 0,0,2,0));

  XSizeAndOffset_HF->AddFrame(DisplayXTitleSize_NEL = new ADAQNumberEntryWithLabel(XSizeAndOffset_HF, "Size", DisplayXSize_NEL_ID),
			      new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DisplayXTitleSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DisplayXTitleSize_NEL->GetEntry()->SetNumber(0.05);
  DisplayXTitleSize_NEL->GetEntry()->Resize(52,20);
  
  XSizeAndOffset_HF->AddFrame(DisplayXTitleOffset_NEL = new ADAQNumberEntryWithLabel(XSizeAndOffset_HF, "Offset", DisplayXOffset_NEL_ID),
			      new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DisplayXTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DisplayXTitleOffset_NEL->GetEntry()->SetNumber(1.2);
  DisplayXTitleOffset_NEL->GetEntry()->Resize(52,20);

  DisplayTitle_GF->AddFrame(DisplayYTitle_TEL = new ADAQTextEntryWithLabel(DisplayTitle_GF, "Y-axis", DisplayYTitle_TEL_ID),
			    new TGLayoutHints(kLHintsNormal,5,5,5,0));
  DisplayYTitle_TEL->GetEntry()->SetText("");

  TGHorizontalFrame *YSizeAndOffset_HF = new TGHorizontalFrame(DisplayTitle_GF);
  DisplayTitle_GF->AddFrame(YSizeAndOffset_HF, new TGLayoutHints(kLHintsNormal, 0,0,2,0));
  
  YSizeAndOffset_HF->AddFrame(DisplayYTitleSize_NEL = new ADAQNumberEntryWithLabel(YSizeAndOffset_HF, "Size", DisplayYSize_NEL_ID),
			    new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DisplayYTitleSize_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DisplayYTitleSize_NEL->GetEntry()->SetNumber(0.05);
  DisplayYTitleSize_NEL->GetEntry()->Resize(52,20);

  YSizeAndOffset_HF->AddFrame(DisplayYTitleOffset_NEL = new ADAQNumberEntryWithLabel(YSizeAndOffset_HF, "Offset", DisplayYOffset_NEL_ID),
			    new TGLayoutHints(kLHintsNormal,5,5,0,5));
  DisplayYTitleOffset_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DisplayYTitleOffset_NEL->GetEntry()->SetNumber(1.5);
  DisplayYTitleOffset_NEL->GetEntry()->Resize(52,20);

  SetTitlesWidgetState(false, kButtonDisabled);
    

  /////////////////////////////
  // misc. graphical attributes 
  
  TGGroupFrame *DisplaySettings_GF = new TGGroupFrame(GraphicsSubframe, "Display", kVerticalFrame);
  DisplaySettings_GF->SetTitlePos(TGGroupFrame::kCenter);
  GraphicsSubframe->AddFrame(DisplaySettings_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  // ROOT check button to enable/disable plotting of the legend
  DisplaySettings_GF->AddFrame(DisplayLegend_CB = new TGCheckButton(DisplaySettings_GF, "Draw legend", -1),
			       new TGLayoutHints(kLHintsNormal,0,0,5,0));
  DisplayLegend_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  DisplaySettings_GF->AddFrame(DisplayGrid_CB = new TGCheckButton(DisplaySettings_GF, "Draw grid", DisplayGrid_CB_ID),
			       new TGLayoutHints(kLHintsNormal, 0,0,0,5));
  DisplayGrid_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DisplayGrid_CB->SetState(kButtonDown);

  // ROOT check buttons for specifying if X and Y axes on spectra should be logarithmic
  DisplaySettings_GF->AddFrame(DisplayXAxisLog_CB = new TGCheckButton(DisplaySettings_GF, "Log X-axis  ", DisplayXAxisLog_CB_ID),
			       new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DisplayXAxisLog_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  DisplaySettings_GF->AddFrame(DisplayYAxisLog_CB = new TGCheckButton(DisplaySettings_GF, "Log Y-axis", DisplayYAxisLog_CB_ID),
			       new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DisplayYAxisLog_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");

  DisplaySettings_GF->AddFrame(SpectrumRefreshRate_NEL = new ADAQNumberEntryWithLabel(DisplaySettings_GF, "Spectrum refresh rate", -1),
			       new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  SpectrumRefreshRate_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumRefreshRate_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  SpectrumRefreshRate_NEL->GetEntry()->SetNumber(100);


  //////////////////
  // Data storage //
  //////////////////

  TGGroupFrame *WaveformStorage_GF = new TGGroupFrame(DataSubframe, "Waveform storage", kVerticalFrame);
  WaveformStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DataSubframe->AddFrame(WaveformStorage_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  WaveformStorage_GF->AddFrame(WaveformFileName_TB = new TGTextButton(WaveformStorage_GF, "Waveform data file name", WaveformFileName_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,5,0));
  WaveformFileName_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformFileName_TB->Resize(175, 30);
  WaveformFileName_TB->ChangeOptions(WaveformFileName_TB->GetOptions() | kFixedSize);

  WaveformStorage_GF->AddFrame(WaveformFileName_TEL = new ADAQTextEntryWithLabel(WaveformStorage_GF, "", -1),
				      new TGLayoutHints(kLHintsNormal,10,5,5,0));
  WaveformFileName_TEL->GetEntry()->Resize(175, 25);
  WaveformFileName_TEL->GetEntry()->ChangeOptions(WaveformFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  WaveformFileName_TEL->GetEntry()->SetState(false);
  WaveformFileName_TEL->GetEntry()->SetText("DefaultData.adaq");

  // ROOT text button to create a root file using the name in the text entry field above
  WaveformStorage_GF->AddFrame(WaveformCreateFile_TB = new TGTextButton(WaveformStorage_GF,"Create ADAQ file", WaveformCreateFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,8,5));
  WaveformCreateFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformCreateFile_TB->Resize(175,30);
  WaveformCreateFile_TB->ChangeOptions(WaveformCreateFile_TB->GetOptions() | kFixedSize);
  WaveformCreateFile_TB->SetState(kButtonDisabled);

  // ROOT text button to write all data to the ROOT file and close it. This button MUST be clicked to 
  // successfully write&close the ROOT file otherwise the ROOT file will have errors.
  WaveformStorage_GF->AddFrame(WaveformCloseFile_TB = new TGTextButton(WaveformStorage_GF,"Close ADAQ file", WaveformCloseFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,0,5));
  WaveformCloseFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformCloseFile_TB->Resize(175,30);
  WaveformCloseFile_TB->ChangeOptions(WaveformCloseFile_TB->GetOptions() | kFixedSize);
  WaveformCloseFile_TB->SetState(kButtonDisabled);
  
  // ROOT check button to enable/disable saving data to ROOT file. Note that the data is saved to
  // the ROOT file only while the button is checked. The 
  WaveformStorage_GF->AddFrame(WaveformStorageEnable_CB = new TGCheckButton(WaveformStorage_GF,"Data stored while checked", WaveformStorageEnable_CB_ID),
			       new TGLayoutHints(kLHintsNormal,10,5,5,5));
  WaveformStorageEnable_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  WaveformStorageEnable_CB->SetState(kButtonDisabled);
  
  
  DGDisplayAndControls_VF->AddFrame(Display_VF, new TGLayoutHints(kLHintsCenterX,5,5,5,5));
  DGDisplayAndControls_VF->AddFrame(SubtabFrame, new TGLayoutHints(kLHintsCenterX,5,5,5,5));

  AcquisitionFrame->AddFrame(DGDisplayAndControls_VF, new TGLayoutHints(kLHintsNormal,5,5,0,10));
  DGDisplayAndControls_VF->SetBackgroundColor(ColorManager->Number2Pixel(22));


  // Widgets for saving the spectrum data to file

  TGGroupFrame *DGScopeSpectrumStorage_GF = new TGGroupFrame(DataSubframe, "Spectrum output", kVerticalFrame);
  DGScopeSpectrumStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DataSubframe->AddFrame(DGScopeSpectrumStorage_GF, new TGLayoutHints(kLHintsNormal,0,5,5,5));

  DGScopeSpectrumStorage_GF->AddFrame(SpectrumFileName_TB = new TGTextButton(DGScopeSpectrumStorage_GF, "Spectrum file name", SpectrumFileName_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SpectrumFileName_TB->Resize(175, 30);
  SpectrumFileName_TB->ChangeOptions(SpectrumFileName_TB->GetOptions() | kFixedSize);
  SpectrumFileName_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
  DGScopeSpectrumStorage_GF->AddFrame(SpectrumFileName_TEL = new ADAQTextEntryWithLabel(DGScopeSpectrumStorage_GF, "", -1),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SpectrumFileName_TEL->GetEntry()->Resize(175, 25);
  SpectrumFileName_TEL->GetEntry()->ChangeOptions(SpectrumFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  SpectrumFileName_TEL->GetEntry()->SetState(false);
  SpectrumFileName_TEL->GetEntry()->SetText("DefaultSpectrum.dat");

  DGScopeSpectrumStorage_GF->AddFrame(SpectrumSaveWithTimeExtension_CB = new TGCheckButton(DGScopeSpectrumStorage_GF, "Add time to file name", SpectrumSaveWithTimeExtension_CB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeSpectrumStorage_GF->AddFrame(SpectrumSave_TB = new TGTextButton(DGScopeSpectrumStorage_GF, "Save spectrum data", SpectrumSave_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  SpectrumSave_TB->Resize(175, 30);
  SpectrumSave_TB->ChangeOptions(SpectrumSave_TB->GetOptions() | kFixedSize);
  SpectrumSave_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");


  // Widgets for saving the canvas graphics to file
  
  TGGroupFrame *DGScopeCanvasStorage_GF = new TGGroupFrame(DataSubframe, "Canvas output", kVerticalFrame);
  DGScopeCanvasStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DataSubframe->AddFrame(DGScopeCanvasStorage_GF, new TGLayoutHints(kLHintsNormal,0,0,5,5));

  DGScopeCanvasStorage_GF->AddFrame(CanvasFileName_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Canvas file name", CanvasFileName_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  CanvasFileName_TB->Resize(175, 30);
  CanvasFileName_TB->ChangeOptions(CanvasFileName_TB->GetOptions() | kFixedSize);
  CanvasFileName_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
  DGScopeCanvasStorage_GF->AddFrame(CanvasFileName_TEL = new ADAQTextEntryWithLabel(DGScopeCanvasStorage_GF, "", -1),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  CanvasFileName_TEL->GetEntry()->Resize(175, 25);
  CanvasFileName_TEL->GetEntry()->ChangeOptions(CanvasFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  CanvasFileName_TEL->GetEntry()->SetState(false);
  CanvasFileName_TEL->GetEntry()->SetText("DefaultGraphics.eps");

  DGScopeCanvasStorage_GF->AddFrame(CanvasSaveWithTimeExtension_CB = new TGCheckButton(DGScopeCanvasStorage_GF, "Add time to file name", CanvasSaveWithTimeExtension_CB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeCanvasStorage_GF->AddFrame(CanvasSave_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Save canvas graphics", CanvasSave_TB_ID),
				    new TGLayoutHints(kLHintsNormal,5,5,0,5));
  CanvasSave_TB->Resize(175, 30);
  CanvasSave_TB->ChangeOptions(CanvasSave_TB->GetOptions() | kFixedSize);
  CanvasSave_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
}
  

// Perform actions that ensure a safe shutdown and disconnect of the
// AAInterface software from the VME boards
void AAInterface::HandleDisconnectAndTerminate(bool Terminate)
{
  TheVMEManager->SafelyDisconnectVMEBoards();
  
  if(Terminate)
    gApplication->Terminate();
}


void AAInterface::SetVoltageChannelWidgetState(int HVChannel, bool HVActive)
{
  bool WidgetState = true;
  if(HVActive)
    WidgetState = false;
  
  HVChVoltage_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
  HVChCurrent_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
}


void AAInterface::SetVoltageWidgetState(bool WidgetState, EButtonState ButtonState)
{
  for(int ch=0; ch<6; ch++){
    HVChVoltage_NEL[ch]->GetEntry()->SetState(WidgetState);
    HVChCurrent_NEL[ch]->GetEntry()->SetState(WidgetState);

    HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(WidgetState);
    HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(WidgetState);
    
    HVChPower_TB[ch]->SetState(ButtonState);
  }
  HVMonitorEnable_CB->SetState(ButtonState);
}


void AAInterface::SetAcquisitionWidgetState(bool WidgetState, EButtonState ButtonState)
{
  // Widgets that have straightforward on/off behavoir depending on
  // the acquisition state

  for(uint32_t ch=0; ch<8; ch++){
    DGChEnable_CB[ch]->SetState(ButtonState);
    DGChPosPolarity_RB[ch]->SetState(ButtonState);
    DGChNegPolarity_RB[ch]->SetState(ButtonState);
    DGChDCOffset_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChZSThreshold_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChZSBackward_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChZSForward_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChZSPosLogic_RB[ch]->SetState(ButtonState);
    DGChZSNegLogic_RB[ch]->SetState(ButtonState);
  }

  AQWaveform_RB->SetEnabled(WidgetState);
  AQSpectrum_RB->SetEnabled(WidgetState);
  AQHighRate_RB->SetEnabled(WidgetState);
  AQUltraRate_RB->SetEnabled(WidgetState);
  
  DGTriggerType_CBL->GetComboBox()->SetEnabled(WidgetState);
  DGTriggerEdge_CBL->GetComboBox()->SetEnabled(WidgetState);
  DGTriggerCoincidenceEnable_CB->SetState(ButtonState);

  DGRecordLength_NEL->GetEntry()->SetState(WidgetState);
  DGPostTrigger_NEL->GetEntry()->SetState(WidgetState);

  DGEventsBeforeReadout_NEL->GetEntry()->SetState(WidgetState);
  AQDataReductionEnable_CB->SetState(ButtonState);
  AQDataReductionFactor_NEL->GetEntry()->SetState(WidgetState);
  DGZSEnable_CB->SetState(ButtonState);

  SpectrumNumBins_NEL->GetEntry()->SetState(WidgetState);
  SpectrumMinBin_NEL->GetEntry()->SetState(WidgetState);
  SpectrumMaxBin_NEL->GetEntry()->SetState(WidgetState);

  SpectrumPulseHeight_RB->SetEnabled(WidgetState);
  SpectrumPulseArea_RB->SetEnabled(WidgetState);
  SpectrumLLD_NEL->GetEntry()->SetState(WidgetState);
  SpectrumULD_NEL->GetEntry()->SetState(WidgetState);
  SpectrumLDTrigger_CB->SetState(ButtonState);
  SpectrumLDTriggerChannel_CBL->GetComboBox()->SetEnabled(WidgetState);
  
  SpectrumRefreshRate_NEL->GetEntry()->SetState(WidgetState);

  // The following widgets have special settings depending on
  // the acquisition state
  
  // Acquisition is turning ON
  if(WidgetState == false){
    AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(kGreen+2));
    AQStartStop_TB->SetText("Acquiring");
    
    WaveformCreateFile_TB->SetState(kButtonUp);
    WaveformCloseFile_TB->SetState(kButtonUp);
    WaveformStorageEnable_CB->SetState(kButtonUp);
  }

  // Acquisition is turning OFF
  else{
    AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
    AQStartStop_TB->SetText("Stopped");

    WaveformCreateFile_TB->SetState(kButtonDisabled);
    WaveformCloseFile_TB->SetState(kButtonDisabled);
    WaveformStorageEnable_CB->SetState(kButtonUp);
    WaveformStorageEnable_CB->SetState(kButtonDisabled);

    bool AQTimerEnable = AAAcquisitionManager::GetInstance()->GetAcquisitionTimerEnable();
    
    if(AQTimerEnable){
      AQTimerStart_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
      AQTimerStart_TB->SetText("Start timer");
    }      
  }
}


void AAInterface::SetCalibrationWidgetState(bool WidgetState, EButtonState ButtonState)
{
  SpectrumUseCalibrationSlider_CB->SetState(ButtonState);
  SpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(WidgetState);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(WidgetState);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetState(WidgetState);
  SpectrumCalibrationSetPoint_TB->SetState(ButtonState);
  SpectrumCalibrationCalibrate_TB->SetState(ButtonState);
  SpectrumCalibrationPlot_TB->SetState(ButtonState);
  SpectrumCalibrationReset_TB->SetState(ButtonState);
  SpectrumCalibrationLoad_TB->SetState(ButtonState);
  SpectrumCalibrationWrite_TB->SetState(kButtonUp);
}


void AAInterface::SetTitlesWidgetState(bool WidgetState, EButtonState ButtonState)
{
  DisplayTitle_TEL->GetEntry()->SetState(WidgetState);
  DisplayXTitle_TEL->GetEntry()->SetState(WidgetState);
  DisplayXTitleSize_NEL->GetEntry()->SetState(WidgetState);
  DisplayXTitleOffset_NEL->GetEntry()->SetState(WidgetState);
  DisplayYTitle_TEL->GetEntry()->SetState(WidgetState);
  DisplayYTitleSize_NEL->GetEntry()->SetState(WidgetState);
  DisplayYTitleOffset_NEL->GetEntry()->SetState(WidgetState);
}


void AAInterface::SaveSettings()
{
  // Acquisition channel 
  for(int ch=0; ch<NumDataChannels; ch++){
    TheSettings->ChEnable[ch] = DGChEnable_CB[ch]->IsDown();
    TheSettings->ChPosPolarity[ch] = DGChPosPolarity_RB[ch]->IsDown();
    TheSettings->ChNegPolarity[ch] = DGChNegPolarity_RB[ch]->IsDown();
    TheSettings->ChVertPos[ch] = DGChVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChDCOffset[ch] = DGChDCOffset_NEL[ch]->GetEntry()->GetHexNumber();
    TheSettings->ChTriggerThreshold[ch] = DGChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChBaselineCalcMin[ch] = DGChBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChBaselineCalcMax[ch] = DGChBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChZSThreshold[ch] = DGChZSThreshold_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChZSForward[ch] = DGChZSForward_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChZSBackward[ch] = DGChZSBackward_NEL[ch]->GetEntry()->GetIntNumber();
    TheSettings->ChZSPosLogic[ch] = DGChZSPosLogic_RB[ch]->IsDown();
    TheSettings->ChZSNegLogic[ch] = DGChZSNegLogic_RB[ch]->IsDown();
  }
  
  TheSettings->HorizontalSliderPtr = DisplayHorizontalScale_THS->GetPointerPosition();
  
  float Min, Max;
  
  DisplayHorizontalScale_THS->GetPosition(Min, Max);
  TheSettings->HorizontalSliderMin = Min;
  TheSettings->HorizontalSliderMax = Max;
  
  DisplayVerticalScale_DVS->GetPosition(Min, Max);
  TheSettings->VerticalSliderMin = Min;
  TheSettings->VerticalSliderMax = Max;


  /////////////////////////////
  // Acquisition control subtab

  // Scope display
  TheSettings->WaveformMode = AQWaveform_RB->IsDown();
  TheSettings->SpectrumMode = AQSpectrum_RB->IsDown();
  TheSettings->HighRateMode = AQHighRate_RB->IsDown();
  TheSettings->UltraRateMode = AQUltraRate_RB->IsDown();
    
  // Trigger control settings
  TheSettings->TriggerCoincidenceEnable = DGTriggerCoincidenceEnable_CB->IsDown();
    
  TheSettings->TriggerCoincidenceLevel = DGTriggerCoincidenceLevel_CBL->GetComboBox()->GetSelected();
  TheSettings->TriggerType = DGTriggerType_CBL->GetComboBox()->GetSelected();
  TheSettings->TriggerEdge = DGTriggerEdge_CBL->GetComboBox()->GetSelected();

  // Acquisition
  TheSettings->RecordLength = DGRecordLength_NEL->GetEntry()->GetIntNumber();
  TheSettings->PostTrigger = DGPostTrigger_NEL->GetEntry()->GetIntNumber();
  TheSettings->AcquisitionTime = AQTime_NEL->GetEntry()->GetIntNumber();

  // Readout
  TheSettings->EventsBeforeReadout = DGEventsBeforeReadout_NEL->GetEntry()->GetIntNumber();
  TheSettings->DataReductionEnable = AQDataReductionEnable_CB->IsDown();
  TheSettings->DataReductionFactor = AQDataReductionFactor_NEL->GetEntry()->GetIntNumber();
  TheSettings->ZeroSuppressionEnable = DGZSEnable_CB->IsDown();

  
  ///////////////////////////
  // Spectrum creation subtab

  TheSettings->SpectrumChannel = SpectrumChannel_CBL->GetComboBox()->GetSelected();
  TheSettings->SpectrumNumBins = SpectrumNumBins_NEL->GetEntry()->GetIntNumber();
  TheSettings->SpectrumMinBin = SpectrumMinBin_NEL->GetEntry()->GetIntNumber();
  TheSettings->SpectrumMaxBin = SpectrumMaxBin_NEL->GetEntry()->GetIntNumber();

  TheSettings->SpectrumPulseHeight = SpectrumPulseHeight_RB->IsDown();
  TheSettings->SpectrumPulseArea = SpectrumPulseArea_RB->IsDown();

  TheSettings->LDEnable = SpectrumLDEnable_CB->IsDown();
  TheSettings->SpectrumLLD = SpectrumLLD_NEL->GetEntry()->GetIntNumber();
  TheSettings->SpectrumULD = SpectrumULD_NEL->GetEntry()->GetIntNumber();
  TheSettings->LDTrigger = SpectrumLDTrigger_CB->IsDown();

  TheSettings->LDChannel = SpectrumLDTriggerChannel_CBL->GetComboBox()->GetSelected();
  
  TheSettings->SpectrumCalibrationEnable = SpectrumCalibration_CB->IsDown();
  TheSettings->SpectrumCalibrationUseSlider = SpectrumUseCalibrationSlider_CB->IsDown();

  
  //////////////////////////
  // Graphic settings subtab

  TheSettings->DisplayTitlesEnable = DisplayTitlesEnable_CB->IsDown();

  TheSettings->DisplayTitle = DisplayTitle_TEL->GetEntry()->GetText();

  TheSettings->DisplayXTitle = DisplayXTitle_TEL->GetEntry()->GetText();
  TheSettings->DisplayXTitleSize = DisplayXTitleSize_NEL->GetEntry()->GetNumber();
  TheSettings->DisplayXTitleOffset = DisplayXTitleOffset_NEL->GetEntry()->GetNumber();

  TheSettings->DisplayYTitle = DisplayYTitle_TEL->GetEntry()->GetText();
  TheSettings->DisplayYTitleSize = DisplayYTitleSize_NEL->GetEntry()->GetNumber();
  TheSettings->DisplayYTitleOffset = DisplayYTitleOffset_NEL->GetEntry()->GetNumber();
	
  TheSettings->DisplayLegend = DisplayLegend_CB->IsDown();
  TheSettings->DisplayGrid = DisplayGrid_CB->IsDown();
  TheSettings->DisplayXAxisInLog = DisplayXAxisLog_CB->IsDown();
  TheSettings->DisplayYAxisInLog = DisplayYAxisLog_CB->IsDown();
  
  TheSettings->WaveformStorageEnable = WaveformStorageEnable_CB->IsDown();
  TheSettings->SpectrumSaveWithTimeExtension = SpectrumSaveWithTimeExtension_CB->IsDown();
  TheSettings->CanvasSaveWithTimeExtension = CanvasSaveWithTimeExtension_CB->IsDown();

  TheSettings->SpectrumRefreshRate = SpectrumRefreshRate_NEL->GetEntry()->GetIntNumber();
  

  ////////////////////////////
  // Persistent storage subtab

  bool AcquisitionOn = AAAcquisitionManager::GetInstance()->GetAcquisitionEnable();

  if(AcquisitionOn){
    for(int ch=0; ch<NumDataChannels; ch++){
      TheSettings->ChEnable[ch] = DGChEnable_CB[ch]->IsDisabledAndSelected();
      TheSettings->ChPosPolarity[ch] = DGChPosPolarity_RB[ch]->IsDisabledAndSelected();
      TheSettings->ChNegPolarity[ch] = DGChNegPolarity_RB[ch]->IsDisabledAndSelected();
      TheSettings->ChZSPosLogic[ch] = DGChZSPosLogic_RB[ch]->IsDisabledAndSelected();
      TheSettings->ChZSNegLogic[ch] = DGChZSNegLogic_RB[ch]->IsDisabledAndSelected();
    }

    TheSettings->WaveformMode = AQWaveform_RB->IsDisabledAndSelected();
    TheSettings->SpectrumMode = AQSpectrum_RB->IsDisabledAndSelected();
    TheSettings->HighRateMode = AQHighRate_RB->IsDisabledAndSelected();
    TheSettings->UltraRateMode = AQUltraRate_RB->IsDisabledAndSelected();

    TheSettings->TriggerCoincidenceEnable = DGTriggerCoincidenceEnable_CB->IsDisabledAndSelected();

    TheSettings->DataReductionEnable = AQDataReductionEnable_CB->IsDisabledAndSelected();
    TheSettings->ZeroSuppressionEnable = DGZSEnable_CB->IsDisabledAndSelected();

    TheSettings->SpectrumPulseHeight = SpectrumPulseHeight_RB->IsDisabledAndSelected();
    TheSettings->SpectrumPulseArea = SpectrumPulseArea_RB->IsDisabledAndSelected();

    TheSettings->LDEnable = SpectrumLDEnable_CB->IsDown();
    TheSettings->LDTrigger = SpectrumLDTrigger_CB->IsDisabledAndSelected();

    TheSettings->SpectrumCalibrationEnable = SpectrumCalibration_CB->IsDisabledAndSelected();
    TheSettings->SpectrumCalibrationUseSlider = SpectrumUseCalibrationSlider_CB->IsDisabledAndSelected();

    TheSettings->WaveformStorageEnable = WaveformStorageEnable_CB->IsDisabledAndSelected();
    TheSettings->SpectrumSaveWithTimeExtension = SpectrumSaveWithTimeExtension_CB->IsDisabledAndSelected();
    TheSettings->CanvasSaveWithTimeExtension = CanvasSaveWithTimeExtension_CB->IsDisabledAndSelected();
  }
}


void AAInterface::UpdateAQTimer(int TimeRemaining)
{ AQTimer_NEFL->GetEntry()->SetNumber(TimeRemaining); }


void AAInterface::UpdateAfterAQTimerStopped(bool ROOTFileOpen)
{
  AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(kRed+1));
  AQStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
  AQStartStop_TB->SetText("Stopped");
  
  SetAcquisitionWidgetState(true, kButtonUp);
  
  // Reset the attributes of the timer start text button
  AQTimerStart_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
  AQTimerStart_TB->SetForegroundColor(ColorManager->Number2Pixel(kBlack));
  AQTimerStart_TB->SetText("Start timer");
  
  WaveformCreateFile_TB->SetState(kButtonDisabled);
  WaveformCloseFile_TB->SetState(kButtonDisabled);
  WaveformStorageEnable_CB->SetState(kButtonUp);
  WaveformStorageEnable_CB->SetState(kButtonDisabled);
}


void AAInterface::UpdateAfterCalibrationPointAdded(int SetPoint)
{
  stringstream SS;
  SS << (SetPoint+1);
  string NewPointLabel = "Calibration point " + SS.str();
  SpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(NewPointLabel.c_str(),SetPoint+1);
  
  // Set the combo box to display the new calibration point...
  SpectrumCalibrationPoint_CBL->GetComboBox()->Select(SetPoint+1);
  
  // ...and set the calibration energy and pulse unit ROOT number
  // entry widgets to their default "0.0" and "1.0" respectively,
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
}


string AAInterface::CreateFileDialog(const char *FileTypes[],
				     EFileDialogMode DialogType)
{
  // Create a new window containing the file save dialog. Set the
  // default directory to the user's present directory
  TGFileInfo FileInformation;
  FileInformation.fFileTypes = FileTypes;
  FileInformation.fIniDir = StrDup(getenv("PWD"));
  new TGFileDialog(gClient->GetRoot(), this, DialogType, &FileInformation);

  string FileName, FileExt;

  if(FileInformation.fFilename == NULL)
    FileName = "NULL";
  else{
    
    // If the user selects "All"/"*.*" option then any entered
    // filename is acceptable (i.e. with/without file extensions, with
    // custom file extension, etc)

    if(FileInformation.fFileTypes[FileInformation.fFileTypeIdx] == "All"){
      FileName = FileInformation.fFilename;
    }

    // Otherwise, the filename will always have the predetermined file
    // extension that is selected in the file type selection list

    else{
      
      // Get the file extension and strip off the leading "*" character
      FileExt = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];
      FileExt = FileExt.erase(0,1);

      // Get the file name
      FileName = FileInformation.fFilename;

      size_t Found = FileName.find_last_of(".");
      
      // If the user has entered a file extension, strip and replace it
      if(Found != string::npos){
	FileName = FileName.substr(0, Found) + FileExt;
      }
      
      // If the user has not entered a file extension, add it
      else{
	FileName += FileExt;
      }
    }
  }
  return FileName;
}
