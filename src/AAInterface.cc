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

AAInterface::AAInterface()
  : TGMainFrame(gClient->GetRoot()),
    DisplayWidth(1125), DisplayHeight(840), 
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

  // Create a pointer to the singleton VME manager for ease-of-use
  TheVMEManager = AAVMEManager::GetInstance();

  
  /////////////////////////////
  // Initialize HV variables //
  /////////////////////////////
  // Initialize C++ stdlib vectors and maps but using the
  // Boost::Assign functionality for its utility and concision

  // std::vector for HV channel labels
  HVChannelLabels += 
    "Channel 0 (-)", "Channel 1 (-)", "Channel 2 (-)", 
    "Channel 3 (+)", "Channel 4 (+)", "Channel 5 (+)";
  
  // std::vector to return the ROOT channel power widget ID from the HV channel number
  HVChannelPower_TB_ID_Vec += 
    (int)HVChannel0Power_TB_ID, (int)HVChannel1Power_TB_ID, (int)HVChannel2Power_TB_ID,
    (int)HVChannel3Power_TB_ID, (int)HVChannel4Power_TB_ID, (int)HVChannel5Power_TB_ID;
  
  // std::map to return the HV channel number from the ROOT channel power widget ID
  insert(HVChannelPower_TB_ID_Map) 
    ((int)HVChannel0Power_TB_ID,0) ((int)HVChannel1Power_TB_ID,1) ((int)HVChannel2Power_TB_ID,2)
    ((int)HVChannel3Power_TB_ID,3) ((int)HVChannel4Power_TB_ID,4) ((int)HVChannel5Power_TB_ID,5);

  /////////////////////////////
  // Initialize DG variables //
  /////////////////////////////

  DGChannelLabels += 
    "Channel 0", "Channel 1", "Channel 2", "Channel 3", 
    "Channel 4", "Channel 5", "Channel 6", "Channel 7";

  DGScopeChEnable_CB_ID_Vec += 
    (int)DGScopeCh0Enable_CB_ID, (int)DGScopeCh1Enable_CB_ID, (int)DGScopeCh2Enable_CB_ID, 
    (int)DGScopeCh3Enable_CB_ID, (int)DGScopeCh4Enable_CB_ID, (int)DGScopeCh5Enable_CB_ID, 
    (int)DGScopeCh6Enable_CB_ID, (int)DGScopeCh7Enable_CB_ID;

  DGScopeChDCOffset_NEL_ID_Vec += 
    (int)DGScopeCh0DCOffset_NEL_ID, (int)DGScopeCh1DCOffset_NEL_ID, (int)DGScopeCh2DCOffset_NEL_ID, 
    (int)DGScopeCh3DCOffset_NEL_ID, (int)DGScopeCh4DCOffset_NEL_ID, (int)DGScopeCh5DCOffset_NEL_ID, 
    (int)DGScopeCh6DCOffset_NEL_ID, (int)DGScopeCh7DCOffset_NEL_ID;

  DGScopeChTriggerThreshold_NEL_ID_Vec += 
    (int)DGScopeCh0TriggerThreshold_NEL_ID, (int)DGScopeCh1TriggerThreshold_NEL_ID, (int)DGScopeCh2TriggerThreshold_NEL_ID, 
    (int)DGScopeCh3TriggerThreshold_NEL_ID, (int)DGScopeCh4TriggerThreshold_NEL_ID, (int)DGScopeCh5TriggerThreshold_NEL_ID, 
    (int)DGScopeCh6TriggerThreshold_NEL_ID, (int)DGScopeCh7TriggerThreshold_NEL_ID;

  DGScopeChBaselineCalcMin_NEL_ID_Vec += 
    (int)DGScopeCh0BaselineCalcMin_NEL_ID, (int)DGScopeCh1BaselineCalcMin_NEL_ID, (int)DGScopeCh2BaselineCalcMin_NEL_ID, 
    (int)DGScopeCh3BaselineCalcMin_NEL_ID, (int)DGScopeCh4BaselineCalcMin_NEL_ID, (int)DGScopeCh5BaselineCalcMin_NEL_ID,
    (int)DGScopeCh6BaselineCalcMin_NEL_ID, (int)DGScopeCh7BaselineCalcMin_NEL_ID;

  DGScopeChBaselineCalcMax_NEL_ID_Vec += 
    (int)DGScopeCh0BaselineCalcMax_NEL_ID, (int)DGScopeCh1BaselineCalcMax_NEL_ID, (int)DGScopeCh2BaselineCalcMax_NEL_ID,
    (int)DGScopeCh3BaselineCalcMax_NEL_ID, (int)DGScopeCh4BaselineCalcMax_NEL_ID, (int)DGScopeCh5BaselineCalcMax_NEL_ID, 
    (int)DGScopeCh6BaselineCalcMax_NEL_ID, (int)DGScopeCh7BaselineCalcMax_NEL_ID;
  
  insert(DGScopeChTriggerThreshold_NEL_ID_Map)
    ((int)DGScopeCh0TriggerThreshold_NEL_ID,0) ((int)DGScopeCh1TriggerThreshold_NEL_ID,1) ((int)DGScopeCh2TriggerThreshold_NEL_ID,2) 
    ((int)DGScopeCh3TriggerThreshold_NEL_ID,3) ((int)DGScopeCh4TriggerThreshold_NEL_ID,4) ((int)DGScopeCh5TriggerThreshold_NEL_ID,5)
    ((int)DGScopeCh6TriggerThreshold_NEL_ID,6) ((int)DGScopeCh7TriggerThreshold_NEL_ID,7);


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
  delete TabSlots;
  delete SubtabSlots;
  delete DisplaySlots;
  delete ChannelSlots;
}


void AAInterface::CreateTopLevelFrames()
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
  VMEConnect_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
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
  AddressTitle += "V1718 base address", "V1720 base address", "V6534 base address";

  vector<int> BoardEnableID, BoardAddressID, BoardAddress;
  BoardEnableID += (int)V1718BoardEnable_TB_ID, (int)V1720BoardEnable_TB_ID, (int)V6534BoardEnable_TB_ID;
  BoardAddressID += (int)0, (int)V1720BoardAddress_ID, (int)V6534BoardAddress_ID;

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
    BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(8));
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
}


void AAInterface::FillRegisterFrame()
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
  
  // The widgets and layout for each channel are identical although
  // the underlying functionality must provide for setting the
  // appropriate V6534 registers for each unique channel. Thus,
  // building and laying out the HV widgets is handled in a "for"
  // loop, with previously initialized std::vectors and std::maps used
  // to retrieve unique values for each channel where necessary. Note:
  // widget IDs are set to "-1" when the widget ID is not explicity
  // needed in the code.


  const int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
  
  for(int ch=0; ch<HVChannels; ch++){
    
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
    HVChannelV_NEL[ch]->GetEntry()->SetLimitValues(0, TheVMEManager->GetHVManager()->GetMaxVoltage());
    HVChannelV_NEL[ch]->GetEntry()->SetNumber(0);
    
    // ADAQ number entry for setting maximum channel current that can be drawn
    HVChannelSet_VF->AddFrame(HVChannelI_NEL[ch] = new ADAQNumberEntryWithLabel(HVChannelSet_VF, "Set Current [uA]",-1),
			      new TGLayoutHints(kLHintsTop | kLHintsLeft, 5,5,5,0));
    HVChannelI_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    HVChannelI_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    HVChannelI_NEL[ch]->GetEntry()->SetLimitValues(0, TheVMEManager->GetHVManager()->GetMaxCurrent());
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
    HVChannelPower_TB[ch]->Connect("Pressed()", "AATabSlots", TabSlots, "HandleVoltageTextButtons()");
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
  HVMonitorEnable_CB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleVoltageTextButtons()");
  HVMonitorEnable_CB->SetState(kButtonUp);
  
  VoltageFrame->AddFrame(HVChannelControls_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));
}


void AAInterface::FillAcquisitionFrame()
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
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(2000);
    DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->Resize(65,20);
    
    // ADAQ number entry to set minimum sample for baseline calculation [sample]
    DGScopeChannelControl_GF->AddFrame(DGScopeBaselineCalcMin_NEL[ch] = new ADAQNumberEntryWithLabel(DGScopeChannelControl_GF, "Baseline min. (sample)", DGScopeChBaselineCalcMin_NEL_ID_Vec[ch]),
				       new TGLayoutHints(kLHintsNormal));
    DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
    
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
  DGScopeStartStop_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");
  DGScopeStartStop_TB->Resize(300,30);
  DGScopeStartStop_TB->ChangeOptions(DGScopeStartStop_TB->GetOptions() | kFixedSize);
  DGScopeStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(2));
  DGScopeStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(1));


  // ROOT text button for manually triggering of DGScope acquisition
  DGScopeDisplayButtons_HF->AddFrame(DGScopeTrigger_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Manual trigger", DGScopeTrigger_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  DGScopeTrigger_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");
  DGScopeTrigger_TB->Resize(175,30);
  DGScopeTrigger_TB->ChangeOptions(DGScopeTrigger_TB->GetOptions() | kFixedSize);
  
  DGScopeDisplayButtons_HF->AddFrame(DGScopeUpdatePlot_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Update plot", DGScopeUpdatePlot_TB_ID),
				     new TGLayoutHints(kLHintsCenterX, 5,5,0,0));
  DGScopeUpdatePlot_TB->Resize(175,30);
  DGScopeUpdatePlot_TB->ChangeOptions(DGScopeUpdatePlot_TB->GetOptions() | kFixedSize);
  DGScopeUpdatePlot_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");

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

  DGScopeUltraRate_RB = new TGRadioButton(DGScopeMode_BG, "Ultra-rate (non-updateable)", DGScopeUltraRate_RB_ID);
  
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
  DGScopeTriggerControls_GF->AddFrame(DGScopeTriggerType_CBL = new ADAQComboBoxWithLabel(DGScopeTriggerControls_GF, "Type", DGScopeTriggerType_CBL_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,5));
  DGScopeTriggerType_CBL->GetComboBox()->AddEntry("External (NIM)",0);
  DGScopeTriggerType_CBL->GetComboBox()->AddEntry("External (TTL)",1);
  DGScopeTriggerType_CBL->GetComboBox()->AddEntry("Automatic",2);
  DGScopeTriggerType_CBL->GetComboBox()->AddEntry("Software",3);
  DGScopeTriggerType_CBL->GetComboBox()->Select(2);
  DGScopeTriggerType_CBL->GetComboBox()->Resize(110,20);
  DGScopeTriggerType_CBL->GetComboBox()->ChangeOptions(DGScopeTriggerType_CBL->GetComboBox()->GetOptions() | kFixedSize);

  
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
  DGScopeAcquisitionTimerStart_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
  DGScopeTimerButtons_HF->AddFrame(DGScopeAcquisitionTimerAbort_TB = new TGTextButton(DGScopeTimerButtons_HF, "Abort timer", DGScopeAcquisitionTimerAbort_TB_ID),
				   new TGLayoutHints(kLHintsNormal, 5,5,0,0));
  DGScopeAcquisitionTimerAbort_TB->Resize(100, 30);
  DGScopeAcquisitionTimerAbort_TB->ChangeOptions(DGScopeAcquisitionTimerAbort_TB->GetOptions() | kFixedSize);
  DGScopeAcquisitionTimerAbort_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  

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
  DGScopeCheckBufferStatus_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
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
  DGScopeSpectrumChannel_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");
  
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
  DGScopeSpectrumAnalysisHeight_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");

  
  DGScopeSpectrumAnalysisArea_RB = new TGRadioButton(DGScopeSpectrumAnalysis_BG, "PAS", DGScopeSpectrumAnalysisArea_RB_ID);
  DGScopeSpectrumAnalysisArea_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
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
  DGScopeSpectrumCalibration_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DGScopeSpectrumCalibration_CB->SetState(kButtonUp);

  
  SpectrumCalibration_HF0->AddFrame(DGScopeSpectrumUseCalibrationSlider_CB = new TGCheckButton(SpectrumCalibration_HF0, "Use slider", DGScopeSpectrumUseCalibrationSlider_CB_ID),
				    new TGLayoutHints(kLHintsLeft,25,5,5,0));
  DGScopeSpectrumUseCalibrationSlider_CB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonDown);
  DGScopeSpectrumUseCalibrationSlider_CB->SetEnabled(false);

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

  SpectrumCalibration_HF1->AddFrame(DGScopeSpectrumCalibrationPoint_CBL = new ADAQComboBoxWithLabel(SpectrumCalibration_HF1, "", DGScopeSpectrumCalibrationPoint_CBL_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,10,3));
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Resize(150,20);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0",0);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(false);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");
  DGScopeSpectrumUseCalibrationSlider_CB->SetState(kButtonDisabled);
  
  SpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationEnergy_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Energy (keV or MeV)", DGScopeSpectrumCalibrationEnergy_NEL_ID),
				   new TGLayoutHints(kLHintsLeft,0,0,0,0));
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->Connect("ValueSet(long)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");

  SpectrumCalibration_GF->AddFrame(DGScopeSpectrumCalibrationPulseUnit_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_GF, "Pulse unit (ADC)", DGScopeSpectrumCalibrationPulseUnit_NEL_ID),
				       new TGLayoutHints(kLHintsLeft,0,0,0,5));
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(false);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->Connect("ValueSet(long)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");

  TGHorizontalFrame *SpectrumCalibration_HF2 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF2);
  
  // Set point text button
  SpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationSetPoint_TB = new TGTextButton(SpectrumCalibration_HF2, "Set Pt.", DGScopeSpectrumCalibrationSetPoint_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationSetPoint_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeSpectrumCalibrationSetPoint_TB->Resize(100,25);
  DGScopeSpectrumCalibrationSetPoint_TB->ChangeOptions(DGScopeSpectrumCalibrationSetPoint_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationSetPoint_TB->SetState(kButtonDisabled);

  // Calibrate text button
  SpectrumCalibration_HF2->AddFrame(DGScopeSpectrumCalibrationCalibrate_TB = new TGTextButton(SpectrumCalibration_HF2, "Calibrate", DGScopeSpectrumCalibrationCalibrate_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationCalibrate_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeSpectrumCalibrationCalibrate_TB->Resize(100,25);
  DGScopeSpectrumCalibrationCalibrate_TB->ChangeOptions(DGScopeSpectrumCalibrationCalibrate_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationCalibrate_TB->SetState(kButtonDisabled);
  
  TGHorizontalFrame *SpectrumCalibration_HF3 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF3);
  
  // Plot text button
  SpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationPlot_TB = new TGTextButton(SpectrumCalibration_HF3, "Plot", DGScopeSpectrumCalibrationPlot_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationPlot_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeSpectrumCalibrationPlot_TB->Resize(100,25);
  DGScopeSpectrumCalibrationPlot_TB->ChangeOptions(DGScopeSpectrumCalibrationPlot_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationPlot_TB->SetState(kButtonDisabled);

  // Reset text button
  SpectrumCalibration_HF3->AddFrame(DGScopeSpectrumCalibrationReset_TB = new TGTextButton(SpectrumCalibration_HF3, "Reset", DGScopeSpectrumCalibrationReset_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationReset_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeSpectrumCalibrationReset_TB->Resize(100,25);
  DGScopeSpectrumCalibrationReset_TB->ChangeOptions(DGScopeSpectrumCalibrationReset_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationReset_TB->SetState(kButtonDisabled);


  TGHorizontalFrame *SpectrumCalibration_HF4 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF4);

  // Load from file text button
  SpectrumCalibration_HF4->AddFrame(DGScopeSpectrumCalibrationLoad_TB = new TGTextButton(SpectrumCalibration_HF4, "Load", DGScopeSpectrumCalibrationLoad_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeSpectrumCalibrationLoad_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeSpectrumCalibrationLoad_TB->Resize(100,25);
  DGScopeSpectrumCalibrationLoad_TB->ChangeOptions(DGScopeSpectrumCalibrationLoad_TB->GetOptions() | kFixedSize);
  DGScopeSpectrumCalibrationLoad_TB->SetState(kButtonDisabled);

  // Write to file text button
  SpectrumCalibration_HF4->AddFrame(DGScopeSpectrumCalibrationWrite_TB = new TGTextButton(SpectrumCalibration_HF4, "Write", DGScopeSpectrumCalibrationWrite_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  DGScopeSpectrumCalibrationWrite_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
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
  DGScopeDataFileName_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");
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
  DGScopeDataStorageCreateFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGScopeDataStorageCreateFile_TB->Resize(175,30);
  DGScopeDataStorageCreateFile_TB->ChangeOptions(DGScopeDataStorageCreateFile_TB->GetOptions() | kFixedSize);
  DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);

  // ROOT text button to write all data to the ROOT file and close it. This button MUST be clicked to 
  // successfully write&close the ROOT file otherwise the ROOT file will have errors.
  DGScopeDataStorage_GF->AddFrame(DGScopeDataStorageCloseFile_TB = new TGTextButton(DGScopeDataStorage_GF,"Close ADAQ file", DGScopeDataStorageCloseFile_TB_ID),
				  new TGLayoutHints(kLHintsNormal,10,5,0,5));
  DGScopeDataStorageCloseFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
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
  DGScopeSpectrumFileName_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
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
  DGScopeSaveSpectrum_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");


  // Widgets for saving the canvas graphics to file
  
  TGGroupFrame *DGScopeCanvasStorage_GF = new TGGroupFrame(DGScopeDataStorageFrame, "Canvas output", kVerticalFrame);
  DGScopeCanvasStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DGScopeDataStorageFrame->AddFrame(DGScopeCanvasStorage_GF, new TGLayoutHints(kLHintsNormal,0,0,5,5));

  DGScopeCanvasStorage_GF->AddFrame(DGScopeCanvasFileName_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Canvas file name", DGScopeCanvasFileName_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGScopeCanvasFileName_TB->Resize(175, 30);
  DGScopeCanvasFileName_TB->ChangeOptions(DGScopeCanvasFileName_TB->GetOptions() | kFixedSize);
  DGScopeCanvasFileName_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
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
  DGScopeSaveCanvas_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  

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
  
  HVChannelV_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
  HVChannelI_NEL[HVChannel]->GetEntry()->SetState(WidgetState);
}


void AAInterface::SetVoltageWidgetState(bool WidgetState, EButtonState ButtonState)
{
  for(int ch=0; ch<6; ch++){
    HVChannelV_NEL[ch]->GetEntry()->SetState(WidgetState);
    HVChannelI_NEL[ch]->GetEntry()->SetState(WidgetState);

    HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(WidgetState);
    HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(WidgetState);
    
    HVChannelPower_TB[ch]->SetState(ButtonState);
  }
  
  HVMonitorEnable_CB->SetState(ButtonState);
}


void AAInterface::SetAcquisitionWidgetState(bool WidgetState, EButtonState ButtonState)
{
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
  DGScopeTriggerType_CBL->GetComboBox()->SetEnabled(WidgetState);

  DGScopeRecordLength_NEL->GetEntry()->SetState(WidgetState);
  DGScopePostTriggerSize_NEL->GetEntry()->SetState(WidgetState);

  DGScopeWaveform_RB->SetEnabled(WidgetState);
  DGScopeSpectrum_RB->SetEnabled(WidgetState);
  DGScopeHighRate_RB->SetEnabled(WidgetState);
  DGScopeUltraRate_RB->SetEnabled(WidgetState);

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
  
  if(TheVMEManager->GetDGAcquisitionEnable())
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
  else
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
  
  DGScopeSpectrumRefreshRate_NEL->GetEntry()->SetState(WidgetState);
}


void AAInterface::SetCalibrationWidgetState(bool WidgetState, EButtonState ButtonState)
{
  DGScopeSpectrumUseCalibrationSlider_CB->SetState(ButtonState);
  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->SetEnabled(WidgetState);
  DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetState(WidgetState);
  DGScopeSpectrumCalibrationSetPoint_TB->SetState(ButtonState);
  DGScopeSpectrumCalibrationCalibrate_TB->SetState(ButtonState);
  DGScopeSpectrumCalibrationPlot_TB->SetState(ButtonState);
  DGScopeSpectrumCalibrationReset_TB->SetState(ButtonState);
  DGScopeSpectrumCalibrationLoad_TB->SetState(ButtonState);
  DGScopeSpectrumCalibrationWrite_TB->SetState(kButtonUp);
}


void AAInterface::SaveSettings()
{
  AASettings *DGSettings = new AASettings;
  
  for(int ch=0; ch<NumDataChannels; ch++){
    DGSettings->ChEnable[ch] = DGScopeChannelEnable_CB[ch]->IsDown();
    DGSettings->ChPosPolarity[ch] = DGScopeChannelPosPolarity_RB[ch]->IsDown();
    DGSettings->ChNegPolarity[ch] = DGScopeChannelNegPolarity_RB[ch]->IsDown();
    DGSettings->ChVertPos[ch] = DGScopeVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChDCOffset[ch] = DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber();
    DGSettings->ChTriggerThreshold[ch] = DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChBaselineCalcMin[ch] = DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChBaselineCalcMax[ch] = DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChZSThreshold[8] = DGScopeZSThreshold_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChZSForward[8] = DGScopeZSForward_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChZSBackward[8] = DGScopeZSBackward_NEL[ch]->GetEntry()->GetIntNumber();
    DGSettings->ChZSPosLogic[8] = DGScopeZSPosLogic_RB[ch]->IsDown();
    DGSettings->ChZSNegLogic[8] = DGScopeZSNegLogic_RB[ch]->IsDown();
  }
  
  DGSettings->WaveformMode = DGScopeWaveform_RB->IsDown();
  DGSettings->SpectrumMode = DGScopeSpectrum_RB->IsDown();
  DGSettings->HighRateMode = DGScopeHighRate_RB->IsDown();
  DGSettings->UltraRateMode = DGScopeUltraRate_RB->IsDown();

  DGSettings->TriggerCoincidenceEnable = DGScopeTriggerCoincidenceEnable_CB->IsDown();
  DGSettings->TriggerCoincidenceLevel = DGScopeTriggerCoincidenceLevel_CBL->GetComboBox()->GetSelected();
  DGSettings->TriggerType = DGScopeTriggerType_CBL->GetComboBox()->GetSelected();
  DGSettings->TriggerEdge = DGScopeTriggerEdge_CBL->GetComboBox()->GetSelected();

  




}
