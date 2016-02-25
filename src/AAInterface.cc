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


AAInterface::AAInterface(Bool_t ALS, string SFN)
  : TGMainFrame(gClient->GetRoot()),
    InterfaceBuildComplete(false),
    DisplayWidth(1121), DisplayHeight(833), 
    ButtonForeColor(kWhite), ButtonBackColorOn(kGreen-5), ButtonBackColorOff(kRed-3),
    SettingsFileName("DefaultSettings.acq.root"),
    AutoSaveSettings(false), AutoLoadSettings(false),
    NumBoards(3),
    ColorManager(new TColor)
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
  
  // Pass a pointer to this class instance to the acquisition manager
  // so that the GUI can be accessed from there
  AAAcquisitionManager::GetInstance()->SetInterfacePointer(this);
  
  BuildPrimaryFrames();
  
  // If the user has specified an ADAQAcquisition settings file as the
  // first cmd line arg then enable auto load and set the file name
  
  if(ALS){
    // Set the auto-load boolean. This ensures that the settings
    // contained in the file will be automatically loaded when the
    // user establishes a connection to the device(s)
    AutoLoadSettings = true;
    
    // Set the class member string containg the file name
    SettingsFileName = SFN;
    
    // Update the text entry widget with the file name
    SettingsFileName_TEL->GetEntry()->SetText(SettingsFileName.c_str());
    
    // Load the settings
    LoadSettingsFromFile();
  }
}


AAInterface::~AAInterface()
{
  delete TabSlots;
  delete SubtabSlots;
  delete DisplaySlots;
  delete ChannelSlots;
  delete ColorManager;
}


void AAInterface::BuildPrimaryFrames()
{
  // Create the top-level frames, which include the main frame and
  // the major tabs for different uses of the DAQ devices
  CreateTopLevelFrames();
  
  // Fill the settings frame, which contains GUI widets that control
  // global settings for ADAQAcquisition behavior
  FillSettingsFrame();
  
  // Fill the connection frame, which contains GUI widgets for
  // establishing connection to the DAQ devices
  FillConnectionFrame();

  // Set cleanup upon destruction
  SetCleanup(kDeepCleanup);
  
  // Connect main window buttons to close properly
  Connect("CloseWindow()", "AAInterface", this, "HandleDisconnectAndTerminate()");

  // Set the main window title
  string TitleString;
  if(VersionString == "Development")
    TitleString = "ADAQAcquisition (Development version)               Fear is the mind-killer.";
  else
    TitleString = "ADAQAcquisition (Production version " + VersionString + ")               Fear is the mind-killer.";
  SetWindowName(TitleString.c_str());

  // Set the ain window size
  Resize(DisplayWidth, DisplayHeight);
  
  // Map the GUIs onto the main window frame
  MapSubwindows();
  MapWindow();
}


void AAInterface::BuildSecondaryFrames()
{
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  AAAcquisitionManager *TheACQManager = AAAcquisitionManager::GetInstance();
  AAGraphics *TheGRPManager = AAGraphics::GetInstance();
  
  // Fill the secondary frames depending on the established links to
  // the to the requested devices

  FillRegisterFrame();

  Int_t NumHVChannels = 0, NumDGChannels = 0;
  
  if(TheVMEManager->GetBRLinkOpen())
    FillPulserFrame();

  if(TheVMEManager->GetHVLinkOpen()){
    NumHVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    FillVoltageFrame();
  }
  
  if(TheVMEManager->GetDGLinkOpen()){
    FillAcquisitionFrame();
    NumDGChannels = TheVMEManager->GetDGManager()->GetNumChannels();
    
    // Send critical pointers to the graphics manager
    TheGRPManager->SetCanvasPointer(DisplayCanvas_EC->GetCanvas());
  }
  
  // Create a high voltage and channel number-dependent AASettings
  // object. This object is used throughout the code to access
  // widget values and to save/load widget values from disk
  
  TheSettings = new AASettings(NumHVChannels, NumDGChannels);
  
  // Pass the settings pointer to the managers
  TheVMEManager->SetSettingsPointer(TheSettings);
  TheACQManager->SetSettingsPointer(TheSettings);
  TheGRPManager->SetSettingsPointer(TheSettings);

  
  // Set the boolean to register that the GUI build is complete
  InterfaceBuildComplete = true;
  
  // If the user specified an ADAQAcquisition settings file at startup
  // then configure all interface widgets now that they are built
  if(AutoLoadSettings)
    LoadSettingsFromFile();
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
  
  SettingsTab = TopLevelTabs->AddTab(" Settings ");
  SettingsFrame = new TGCompositeFrame(SettingsTab, 60, 20, kHorizontalFrame);
  SettingsTab->AddFrame(SettingsFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  
  ConnectionTab = TopLevelTabs->AddTab(" VME Connection ");
  ConnectionFrame = new TGCompositeFrame(ConnectionTab, 60, 20, kVerticalFrame);
  ConnectionTab->AddFrame(ConnectionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  
  RegisterTab = TopLevelTabs->AddTab(" Register R/W ");
  RegisterFrame = new TGCompositeFrame(RegisterTab, 60, 20, kVerticalFrame);
  RegisterTab->AddFrame(RegisterFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  
  PulserTab = TopLevelTabs->AddTab(" Pulsers ");
  PulserFrame = new TGCompositeFrame(PulserTab, 60, 20, kVerticalFrame);
  PulserTab->AddFrame(PulserFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));

  VoltageTab = TopLevelTabs->AddTab(" High Voltage ");
  VoltageFrame = new TGCompositeFrame(VoltageTab, 60, 20, kHorizontalFrame);
  VoltageTab->AddFrame(VoltageFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,40,5));

  AcquisitionTab = TopLevelTabs->AddTab(" Acquisition ");
  AcquisitionTab->SetBackgroundColor(ColorManager->Number2Pixel(22));
  AcquisitionFrame = new TGCompositeFrame(AcquisitionTab, 60, 20, kHorizontalFrame);
  AcquisitionFrame->SetBackgroundColor(ColorManager->Number2Pixel(22));
  AcquisitionTab->AddFrame(AcquisitionFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
  
  // Set the default tab
  TopLevelTabs->SetTab(" VME Connection ");
  
  // Add the tabs to the frame
  TabFrame->AddFrame(TopLevelTabs, new TGLayoutHints(kLHintsTop, 5,5,5,5));
  

  ////////////////////////////////////////////
  // Add top level frames to the main frame //
  ////////////////////////////////////////////
  
  TopFrame->AddFrame(TabFrame, new TGLayoutHints(kLHintsCenterX, 35,5,5,5));

  AddFrame(TopFrame, new TGLayoutHints(kLHintsExpandX, 0,0,0,0));

  SetBackgroundColor(ColorManager->Number2Pixel(22));
}


void AAInterface::FillSettingsFrame()
{
  TGGroupFrame *SettingsFile_GF = new TGGroupFrame(SettingsFrame, "Settings storage file", kVerticalFrame);
  SettingsFile_GF->SetTitlePos(TGGroupFrame::kCenter);
  SettingsFrame->AddFrame(SettingsFile_GF, new TGLayoutHints(kLHintsLeft, 5,5,5,5));
  
  SettingsFile_GF->AddFrame(SetSettingsFileName_TB = new TGTextButton(SettingsFile_GF,
								      "Set file name",
								      SetSettingsFileName_TB_ID),
			    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SetSettingsFileName_TB->Resize(210, 30);
  SetSettingsFileName_TB->ChangeOptions(SetSettingsFileName_TB->GetOptions() | kFixedSize);
  SetSettingsFileName_TB->Connect("Clicked()", "AATabSlots", SubtabSlots, "HandleSettingsTextButtons()");
  
  SettingsFile_GF->AddFrame(SettingsFileName_TEL = new ADAQTextEntryWithLabel(SettingsFile_GF,
									      "",
									      SettingsFileName_TEL_ID),
			    new TGLayoutHints(kLHintsNormal, 10,5,5,0));
  SettingsFileName_TEL->GetEntry()->Resize(210, 25);
  SettingsFileName_TEL->GetEntry()->ChangeOptions(SettingsFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  SettingsFileName_TEL->GetEntry()->SetState(false);
  SettingsFileName_TEL->GetEntry()->SetText(SettingsFileName.c_str());

  
  TGHorizontalFrame *SaveLoadSettings_HF = new TGHorizontalFrame(SettingsFile_GF);
  SettingsFile_GF->AddFrame(SaveLoadSettings_HF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  SaveLoadSettings_HF->AddFrame(SaveSettingsToFile_TB = new TGTextButton(SaveLoadSettings_HF,
									 "Save",
									 SaveSettingsToFile_TB_ID),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  SaveSettingsToFile_TB->Resize(100, 30);
  SaveSettingsToFile_TB->ChangeOptions(SaveSettingsToFile_TB->GetOptions() | kFixedSize);
  SaveSettingsToFile_TB->Connect("Clicked()", "AATabSlots", SubtabSlots, "HandleSettingsTextButtons()");

  SaveLoadSettings_HF->AddFrame(LoadSettingsFromFile_TB = new TGTextButton(SaveLoadSettings_HF,
									   "Load",
									   LoadSettingsFromFile_TB_ID),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  LoadSettingsFromFile_TB->Resize(100, 30);
  LoadSettingsFromFile_TB->ChangeOptions(SaveSettingsToFile_TB->GetOptions() | kFixedSize);
  LoadSettingsFromFile_TB->Connect("Clicked()", "AATabSlots", SubtabSlots, "HandleSettingsTextButtons()");
  
  SettingsFile_GF->AddFrame(AutoSaveSettings_CB = new TGCheckButton(SettingsFile_GF,
								    "Auto save settings during session",
								    AutoSaveSettings_CB_ID),
			    new TGLayoutHints(kLHintsNormal, 10,5,5,0));
}


void AAInterface::FillConnectionFrame()
{
  // The main VME connection button
  
  TGGroupFrame *Connection_GF = new TGGroupFrame(ConnectionFrame,"Initiate Connection", kVerticalFrame);
  ConnectionFrame->AddFrame(Connection_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));
  Connection_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  Connection_GF->AddFrame(VMEConnect_TB = new TGTextButton(Connection_GF, "Disconnected: click to connect", VMEConnect_TB_ID),
			    new TGLayoutHints(kLHintsExpandX, 5,5,25,5));
  VMEConnect_TB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleConnectionTextButtons()");
  VMEConnect_TB->Resize(500,40);
  VMEConnect_TB->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
  VMEConnect_TB->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
  VMEConnect_TB->ChangeOptions(VMEConnect_TB->GetOptions() | kFixedSize);

  // A text view window to capture/display std::cout information

  Connection_GF->AddFrame(ConnectionOutput_TV = new TGTextView(Connection_GF, 987, 455, -42),
			  new TGLayoutHints(kLHintsTop | kLHintsExpandX, 15,15,5,25));
  ConnectionOutput_TV->SetBackground(ColorManager->Number2Pixel(18));
  

  // The VME addresses, address display, and enable/disable widgets
  
  vector<string> Title;
  Title += "VME-USB Bridge", "VME/Desktop Digitizer", "VME/Desktop High Voltage";
  
  vector<int> BoardEnableID, BoardAddressID, BoardLinkNumberID;
  BoardEnableID += (int)BRBoardEnable_TB_ID, (int)DGBoardEnable_TB_ID, (int)HVBoardEnable_TB_ID;
  BoardAddressID += (int)0, (int)DGBoardAddress_ID, (int)HVBoardAddress_ID;
  BoardLinkNumberID += (int)0, (int)DGBoardLinkNumber_ID, (int)HVBoardLinkNumber_ID;
  
  vector<uint32_t> BoardAddress, BoardLinkNumber;
  BoardAddress += 0, 0x00420000, 0x42420000;
  BoardLinkNumber += 0, 0, 0;

  TGGroupFrame *DeviceSettings_GF = new TGGroupFrame(ConnectionFrame, "CAEN Device Settings", kHorizontalFrame);
  ConnectionFrame->AddFrame(DeviceSettings_GF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5,5,5,5));
  DeviceSettings_GF->SetTitlePos(TGGroupFrame::kCenter);
  
  for(int board=0; board<NumBoards; board++){
    
    TGVerticalFrame *BoardOptions_VF = new TGVerticalFrame(DeviceSettings_GF);
    DeviceSettings_GF->AddFrame(BoardOptions_VF, 
				new TGLayoutHints(kLHintsCenterX, 35,35,5,5));

    BoardOptions_VF->AddFrame(new TGLabel(BoardOptions_VF, Title[board].c_str()), 
			      new TGLayoutHints(kLHintsCenterX, 5,5,5,0));

    TGHorizontalFrame *BoardOptions_HF = new TGHorizontalFrame(BoardOptions_VF);
    BoardOptions_VF->AddFrame(BoardOptions_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

    BoardType_CBL.push_back(new ADAQComboBoxWithLabel(BoardOptions_HF, "Type", -1));
    BoardOptions_HF->AddFrame(BoardType_CBL[board], new TGLayoutHints(kLHintsCenterX, 22,0,10,5));
    BoardType_CBL[board]->GetComboBox()->Resize(75,20);

    if(board == 0){
      BoardType_CBL[board]->GetComboBox()->AddEntry("V1718", zV1718);
      BoardType_CBL[board]->GetComboBox()->Select(zV1718);
    }
    if(board == 1){
      BoardType_CBL[board]->GetComboBox()->AddEntry("V1720", zV1720);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V1724", zV1724);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V1725", zV1725);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5720", zDT5720);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5730", zDT5730);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790M", zDT5790M);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790N", zDT5790N);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790P", zDT5790P);
      BoardType_CBL[board]->GetComboBox()->Select(zV1725);
    }
    else if(board == 2){
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6533M", zV6533M);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6533N", zV6533N);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6533P", zV6533P);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6534M", zV6534M);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6534N", zV6534N);
      BoardType_CBL[board]->GetComboBox()->AddEntry("V6534P", zV6534P);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790M", zDT5790M);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790N", zDT5790N);
      BoardType_CBL[board]->GetComboBox()->AddEntry("DT5790P", zDT5790P);
      BoardType_CBL[board]->GetComboBox()->Select(zV6534M);
    }

    if(board == 1){
      TGVerticalFrame *FWType_VF = new TGVerticalFrame(BoardOptions_HF);
      BoardOptions_HF->AddFrame(FWType_VF, new TGLayoutHints(kLHintsNormal, 15,0,5,0));
      
      FWType_VF->AddFrame(DGStandardFW_RB = new TGRadioButton(FWType_VF, "STD firmware", DGStandardFW_RB_ID),
			  new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      DGStandardFW_RB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleRadioButtons()");
      
      FWType_VF->AddFrame(DGPSDFW_RB = new TGRadioButton(FWType_VF, "DPP-PSD firmware", DGPSDFW_RB_ID),
			  new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      DGPSDFW_RB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleRadioButtons()");
      // DGStandardFW_RB->SetState(kButtonDown);
      DGPSDFW_RB->SetState(kButtonDown);
    }
    
    TGHorizontalFrame *BoardAddress_HF = new TGHorizontalFrame(BoardOptions_VF);
    BoardOptions_VF->AddFrame(BoardAddress_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

    Int_t LeftOffset = 5;
    if(board == 1)
      LeftOffset = 60;
    
    BoardAddress_HF->AddFrame(new TGLabel(BoardAddress_HF,"0x"), 
			      new TGLayoutHints(kLHintsCenterX, LeftOffset,0,5,5));
    
    // The V1718 USB-VME board differs from the others in that, as the
    // VME controller, it does not an explictly settable VME
    // address. Its VME address is set automatically when a link is
    // established (see the ADAQBridge class of the ADAQ library for
    // details) Thus, there is special handling of the V1718 controls.
    
    if(board == 0){
      // Push back a zero to maintain an array size of 3
      BoardAddress_NEF.push_back(0);

      // Create a placeholder alerting the user that the V1718 VME
      // address is automatically set regardless of connection method
      TGTextEntry *V1718_TE = new TGTextEntry(BoardAddress_HF, "Auto-Set!", 0);
      V1718_TE->SetAlignment(kTextCenterX);
      V1718_TE->Resize(75,20);
      BoardAddress_HF->AddFrame(V1718_TE, new TGLayoutHints(kLHintsCenterX, 5, 5, 0, 0));
      
      BoardAddress_HF->AddFrame(new TGLabel(BoardAddress_HF,"Address"), 
				new TGLayoutHints(kLHintsCenterX, 0,0,3,0));
      
    }
    else{
      BoardAddress_NEF.push_back(new ADAQNumberEntryFieldWithLabel(BoardAddress_HF, 
								   "Address",
								   BoardAddressID[board]));
      BoardAddress_NEF[board]->GetEntry()->SetFormat(TGNumberFormat::kNESHex);
      BoardAddress_NEF[board]->GetEntry()->SetHexNumber(BoardAddress[board]);
      BoardAddress_NEF[board]->GetEntry()->Resize(75,20);
      BoardAddress_HF->AddFrame(BoardAddress_NEF[board], new TGLayoutHints(kLHintsCenterX, 5, 0, 0, 0));
    }
    
    BoardLinkNumber_NEL.push_back(new ADAQNumberEntryWithLabel(BoardOptions_VF, 
							       "USB Link",
							       BoardLinkNumber[board]));
    BoardLinkNumber_NEL[board]->GetEntry()->SetFormat(TGNumberFormat::kNESInteger);
    BoardLinkNumber_NEL[board]->GetEntry()->SetHexNumber(BoardLinkNumber[board]);
    BoardLinkNumber_NEL[board]->GetEntry()->Resize(40,20);
    BoardLinkNumber_NEL[board]->GetEntry()->SetNumber(0);

    LeftOffset = 57;
    if(board == 1)
      LeftOffset = 112;
    
    BoardOptions_VF->AddFrame(BoardLinkNumber_NEL[board], new TGLayoutHints(kLHintsNormal, LeftOffset, 0, 2, 5));
    
    BoardEnable_TB.push_back(new TGTextButton(BoardOptions_VF, "Board enabled", BoardEnableID[board]));
    BoardOptions_VF->AddFrame(BoardEnable_TB[board], new TGLayoutHints(kLHintsCenterX));
    
    BoardEnable_TB[board]->Connect("Clicked()", "AATabSlots", TabSlots, "HandleConnectionTextButtons()");
    BoardEnable_TB[board]->Resize(110,25);
    BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOn));
    BoardEnable_TB[board]->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
    BoardEnable_TB[board]->ChangeOptions(BoardEnable_TB[board]->GetOptions() | kFixedSize);
  }
}


void AAInterface::FillRegisterFrame()
{
  if(InterfaceBuildComplete)
    return;
  
  string FrameTitle[NumBoards] = {"CAEN VME Bridge Module", "CAEN Digitizer Module", "CAEN High Voltage Module"};

  Int_t ReadAddressID[NumBoards] = {BRReadAddress_ID, DGReadAddress_ID, HVReadAddress_ID};
  Int_t ReadValueID[NumBoards] = {BRReadValue_ID, DGReadValue_ID, HVReadValue_ID};

  Int_t WriteAddressID[NumBoards] = {BRWriteAddress_ID, DGWriteAddress_ID, HVWriteAddress_ID};
  Int_t WriteValueID[NumBoards] = {BRWriteValue_ID, DGWriteValue_ID, HVWriteValue_ID};

  Int_t ReadID[NumBoards] = {BRRead_ID, DGRead_ID, HVRead_ID};
  Int_t WriteID[NumBoards] = {BRWrite_ID, DGWrite_ID, HVWrite_ID};

  const Int_t RWButtonX = 250;
  const Int_t RWButtonY = 30;
  const Int_t RWFGColor = ColorManager->Number2Pixel(0);
  const Int_t RWBGColor = ColorManager->Number2Pixel(36);
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  for(Int_t board=0; board<NumBoards; board++){
    
    ////////////////////////////////////////////////////
    // Create the group frame to hold all the subwidgets

    TGGroupFrame *RegisterRW_GF = new TGGroupFrame(RegisterFrame, FrameTitle[board].c_str(), kHorizontalFrame);
    RegisterRW_GF->SetTitlePos(TGGroupFrame::kCenter);
    

    /////////////////////////////////////////////////
    // The register read/write and display widgets //
    /////////////////////////////////////////////////

    TGGroupFrame *ReadCycle_GF = new TGGroupFrame(RegisterRW_GF, "Read cycle", kVerticalFrame);
    
    TGHorizontalFrame *ReadCycleAddress_HF = new TGHorizontalFrame(ReadCycle_GF);
    ReadCycle_GF->AddFrame(ReadCycleAddress_HF, new TGLayoutHints(kLHintsExpandY, 5,5,0,0));
    
    TGLabel *ReadCycle_L1 = new TGLabel(ReadCycleAddress_HF, "Offset address  0x");
    ReadCycle_L1->Resize(130,20);
    ReadCycle_L1->SetTextJustify(kTextRight);
    ReadCycle_L1->ChangeOptions(ReadCycle_L1->GetOptions() | kFixedSize);
    
    ReadCycleAddress_HF->AddFrame(ReadCycle_L1, new TGLayoutHints(kLHintsLeft, 5,0,0,0));
    
    // ROOT number entry field for setting the V6534 register address to read from
    ReadAddress_NEF.push_back(new TGNumberEntryField(ReadCycleAddress_HF, ReadAddressID[board], 0, 
						     TGNumberFormat::kNESHex,
						     TGNumberFormat::kNEAPositive));
    ReadAddress_NEF[board]->Resize(80,20);
    ReadCycleAddress_HF->AddFrame(ReadAddress_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,0,0));


    //////////////////////////////////////////////////////////////////////////////////
    // Create three output displays: decimal (dec), hexidecimal (hex),and binary (bin)

    // Decimal readout display

    TGHorizontalFrame *ReadCycleValue_HF0 = new TGHorizontalFrame(ReadCycle_GF);
    ReadCycle_GF->AddFrame(ReadCycleValue_HF0, new TGLayoutHints(kLHintsExpandY, 5,5,5,0));
    
    TGLabel *ReadCycle_L2 = new TGLabel(ReadCycleValue_HF0,"   Value  0d");
    ReadCycle_L2->Resize(130,20);
    ReadCycle_L2->SetTextJustify(kTextRight);
    ReadCycle_L2->ChangeOptions(ReadCycle_L2->GetOptions() | kFixedSize);
    ReadCycleValue_HF0->AddFrame(ReadCycle_L2, new TGLayoutHints(kLHintsLeft, 5,0,5,0));

    ReadValueDec_NEF.push_back(new TGNumberEntryField(ReadCycleValue_HF0, ReadValueID[board], 0, 
						      TGNumberFormat::kNESInteger,
						      TGNumberFormat::kNEAPositive));
    ReadValueDec_NEF[board]->Resize(80,20);
    ReadValueDec_NEF[board]->SetState(false);
    ReadCycleValue_HF0->AddFrame(ReadValueDec_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,5,0));

    // Hexidecimal readout display
    
    TGHorizontalFrame *ReadCycleValue_HF1 = new TGHorizontalFrame(ReadCycle_GF);
    ReadCycle_GF->AddFrame(ReadCycleValue_HF1, new TGLayoutHints(kLHintsExpandY, 5,5,5,0));
    
    TGLabel *ReadCycle_L3 = new TGLabel(ReadCycleValue_HF1,"          0x");
    ReadCycle_L3->Resize(130,20);
    ReadCycle_L3->SetTextJustify(kTextRight);
    ReadCycle_L3->ChangeOptions(ReadCycle_L3->GetOptions() | kFixedSize);
    ReadCycleValue_HF1->AddFrame(ReadCycle_L3, new TGLayoutHints(kLHintsLeft, 5,0,5,0));
    
    ReadValueHex_NEF.push_back(new TGNumberEntryField(ReadCycleValue_HF1, ReadValueID[board], 0, 
						      TGNumberFormat::kNESHex,
						      TGNumberFormat::kNEAPositive));
    ReadValueHex_NEF[board]->Resize(80,20);
    ReadValueHex_NEF[board]->SetState(false);
    ReadCycleValue_HF1->AddFrame(ReadValueHex_NEF[board], new TGLayoutHints(kLHintsExpandX, 5,5,0,0));

    // Binary readout display

    TGHorizontalFrame *ReadCycleValue_HF2 = new TGHorizontalFrame(ReadCycle_GF);
    ReadCycle_GF->AddFrame(ReadCycleValue_HF2, new TGLayoutHints(kLHintsExpandY, 5,5,0,5));
    
    TGLabel *ReadCycle_L4 = new TGLabel(ReadCycleValue_HF2,"          0b");
    ReadCycle_L4->Resize(130,20);
    ReadCycle_L4->SetTextJustify(kTextRight);
    ReadCycle_L4->ChangeOptions(ReadCycle_L4->GetOptions() | kFixedSize);
    ReadCycleValue_HF2->AddFrame(ReadCycle_L4, new TGLayoutHints(kLHintsLeft, 5,0,5,0));
    
    ReadValueBinary_TE.push_back(new TGTextEntry(ReadCycleValue_HF2, "0000 0000 0000 0000 0000 0000 0000 0000"));
    ReadValueBinary_TE[board]->Resize(250,20);
    ReadValueBinary_TE[board]->SetBackgroundColor(ColorManager->Number2Pixel(18));
    ReadCycleValue_HF2->AddFrame(ReadValueBinary_TE[board], new TGLayoutHints(kLHintsLeft, 5,5,0,0));

    ///////////////////////////
    // The VME read text button

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
    WriteCycle_GF->AddFrame(Write_TB[board], new TGLayoutHints(kLHintsCenterX, 5,5,15,5));

    // Add the write cycle group frame to the hierarchy
    RegisterRW_GF->AddFrame(WriteCycle_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,5));
    
    // Add the top-level group frame to the hierarchy
    Int_t BottomOffset = 30;
    if(board == 2)
      BottomOffset = 0;
    
    RegisterFrame->AddFrame(RegisterRW_GF, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5,5,5,BottomOffset));
  }
  
  MapSubwindows();
  MapWindow();
}


void AAInterface::FillPulserFrame()
{
  if(InterfaceBuildComplete)
    return;
  
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
    V1718PulserPeriod_NEL[pulser]->GetEntry()->SetNumber(250);
  
    PulserSettings_VF->AddFrame(V1718PulserWidth_NEL[pulser] = new ADAQNumberEntryWithLabel(PulserSettings_VF, "Width (number of time units)", -1),
				new TGLayoutHints(kLHintsNormal, 5,5,5,0));
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    V1718PulserWidth_NEL[pulser]->GetEntry()->Resize(XSize,YSize);
    V1718PulserWidth_NEL[pulser]->GetEntry()->SetNumber(20);

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
    V1718PulserStartStop_TB[pulser]->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
    V1718PulserStartStop_TB[pulser]->SetForegroundColor(ColorManager->Number2Pixel(ButtonForeColor));
    V1718PulserStartStop_TB[pulser]->Resize(200,40);
    V1718PulserStartStop_TB[pulser]->ChangeOptions(V1718PulserStartStop_TB[pulser]->GetOptions() | kFixedSize);
    V1718PulserStartStop_TB[pulser]->Connect("Pressed()", "AATabSlots", TabSlots, "HandlePulserTextButtons()");
  }  

  MapSubwindows();
  MapWindow();
}


// The "VoltageFrame" holds ROOT widgets for complete control of the
// HV supply board, including real-time monitoring of each channel's
// active voltage and drawn current. Setting the voltage and current
// for an individual channel are disabled while the channel power is
// on. This may be updated in the future to enable real-time changes.
void AAInterface::FillVoltageFrame()
{
  if(InterfaceBuildComplete)
    return;
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  const int NumHVChannels = TheVMEManager->GetHVManager()->GetNumChannels();

  /////////////////////////////
  // Initialize HV variables //
  /////////////////////////////
  // Initialize C++ stdlib vectors and maps but using the
  // Boost::Assign functionality for its utility and concision
  
  for(int ch=0; ch<NumHVChannels; ch++){
    stringstream SS;
    SS << "Channel " << ch << " (" << TheVMEManager->GetHVManager()->GetPolarityString(ch)
       << ")";
    
    HVChLabels.push_back(SS.str());
  }
  
  // std::vector to return the ROOT channel power widget ID from the HV channel number
  HVChPower_TB_ID_Vec += 
    (int)HVCh0Power_TB_ID, (int)HVCh1Power_TB_ID, (int)HVCh2Power_TB_ID,
    (int)HVCh3Power_TB_ID, (int)HVCh4Power_TB_ID, (int)HVCh5Power_TB_ID;
  
  // std::map to return the HV channel number from the ROOT channel power widget ID
  insert(HVChPower_TB_ID_Map) 
    ((int)HVCh0Power_TB_ID,0) ((int)HVCh1Power_TB_ID,1) ((int)HVCh2Power_TB_ID,2)
    ((int)HVCh3Power_TB_ID,3) ((int)HVCh4Power_TB_ID,4) ((int)HVCh5Power_TB_ID,5);


  /////////////////////////////////
  // Build the high voltage GUIs //
  /////////////////////////////////

  TGVerticalFrame *HVChannelControls_VF = new TGVerticalFrame(VoltageFrame);
  
  for(int ch=0; ch<NumHVChannels; ch++){
    
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
    HVChVoltage_NEL[ch]->GetEntry()->SetLimitValues(0, TheVMEManager->GetHVManager()->GetMaxVoltage(ch));
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
    HVChPower_TB[ch]->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
    HVChPower_TB[ch]->SetForegroundColor(ColorManager->Number2Pixel(ButtonForeColor));
    
    // Modify the widget background to distinguish the negative and positive voltage 
    string Polarity = TheVMEManager->GetHVManager()->GetPolarityString(ch);
    if(Polarity == "+")
      HVChannel_GF->SetBackgroundColor(ColorManager->Number2Pixel(22));
    else if(Polarity == "-")
      HVChannel_GF->SetBackgroundColor(ColorManager->Number2Pixel(16));
  }

  TGGroupFrame *HVAllChannel_GF = new TGGroupFrame(HVChannelControls_VF, "All Channels", kHorizontalFrame);
  HVAllChannel_GF->SetTitlePos(TGGroupFrame::kCenter);
  HVChannelControls_VF->AddFrame(HVAllChannel_GF, new TGLayoutHints(kLHintsCenterX, 5,5,5,0));

  // ROOT check button to enable/disable real-time monitoring of all
  // channel's set voltage and drawn current
  HVAllChannel_GF->AddFrame(HVMonitorEnable_CB = new TGCheckButton(HVAllChannel_GF, "Enable monitoring", HVEnableMonitoring_CB_ID),
			    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  HVMonitorEnable_CB->Connect("Clicked()", "AATabSlots", TabSlots, "HandleCheckButtons()");
  HVMonitorEnable_CB->SetState(kButtonUp);
  
  VoltageFrame->AddFrame(HVChannelControls_VF, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 5, 5, 5, 5));

  MapSubwindows();
  MapWindow();
}

 
void AAInterface::FillAcquisitionFrame()
{
  if(InterfaceBuildComplete)
    return;
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  const int NumDGChannels = TheVMEManager->GetDGManager()->GetNumChannels();
  
  /////////////////////////////
  // Initialize DG variables //
  /////////////////////////////

  stringstream SS;
  
  for(Int_t ch=0; ch<NumDGChannels; ch++){
    SS.str("");
    SS << "Channel " << ch;
    DGChannelLabels.push_back(SS.str());
  }

  DGChEnable_CB_ID_Vec += 
    (int)DGCh0Enable_CB_ID,  (int)DGCh1Enable_CB_ID,  (int)DGCh2Enable_CB_ID, 
    (int)DGCh3Enable_CB_ID,  (int)DGCh4Enable_CB_ID,  (int)DGCh5Enable_CB_ID, 
    (int)DGCh6Enable_CB_ID,  (int)DGCh7Enable_CB_ID,  (int)DGCh8Enable_CB_ID,
    (int)DGCh9Enable_CB_ID,  (int)DGCh10Enable_CB_ID, (int)DGCh11Enable_CB_ID,
    (int)DGCh12Enable_CB_ID, (int)DGCh13Enable_CB_ID, (int)DGCh14Enable_CB_ID,
    (int)DGCh15Enable_CB_ID;
  
  DGChDCOffset_NEL_ID_Vec += 
    (int)DGCh0DCOffset_NEL_ID,  (int)DGCh1DCOffset_NEL_ID,  (int)DGCh2DCOffset_NEL_ID, 
    (int)DGCh3DCOffset_NEL_ID,  (int)DGCh4DCOffset_NEL_ID,  (int)DGCh5DCOffset_NEL_ID, 
    (int)DGCh6DCOffset_NEL_ID,  (int)DGCh7DCOffset_NEL_ID,  (int)DGCh8DCOffset_NEL_ID,
    (int)DGCh9DCOffset_NEL_ID,  (int)DGCh10DCOffset_NEL_ID, (int)DGCh11DCOffset_NEL_ID,
    (int)DGCh12DCOffset_NEL_ID, (int)DGCh13DCOffset_NEL_ID, (int)DGCh14DCOffset_NEL_ID,
    (int)DGCh15DCOffset_NEL_ID;

  DGChTriggerThreshold_NEL_ID_Vec += 
    (int)DGCh0TriggerThreshold_NEL_ID,  (int)DGCh1TriggerThreshold_NEL_ID,  (int)DGCh2TriggerThreshold_NEL_ID, 
    (int)DGCh3TriggerThreshold_NEL_ID,  (int)DGCh4TriggerThreshold_NEL_ID,  (int)DGCh5TriggerThreshold_NEL_ID, 
    (int)DGCh6TriggerThreshold_NEL_ID,  (int)DGCh7TriggerThreshold_NEL_ID,  (int)DGCh8TriggerThreshold_NEL_ID,
    (int)DGCh9TriggerThreshold_NEL_ID,  (int)DGCh10TriggerThreshold_NEL_ID, (int)DGCh11TriggerThreshold_NEL_ID,
    (int)DGCh12TriggerThreshold_NEL_ID, (int)DGCh13TriggerThreshold_NEL_ID, (int)DGCh14TriggerThreshold_NEL_ID,
    (int)DGCh15TriggerThreshold_NEL_ID;

  insert(DGChTriggerThreshold_NEL_ID_Map)
    ((int)DGCh0TriggerThreshold_NEL_ID,0)   ((int)DGCh1TriggerThreshold_NEL_ID,1)   ((int)DGCh2TriggerThreshold_NEL_ID,2) 
    ((int)DGCh3TriggerThreshold_NEL_ID,3)   ((int)DGCh4TriggerThreshold_NEL_ID,4)   ((int)DGCh5TriggerThreshold_NEL_ID,5)
    ((int)DGCh6TriggerThreshold_NEL_ID,6)   ((int)DGCh7TriggerThreshold_NEL_ID,7)   ((int)DGCh8TriggerThreshold_NEL_ID,8)
    ((int)DGCh9TriggerThreshold_NEL_ID,9)   ((int)DGCh10TriggerThreshold_NEL_ID,10) ((int)DGCh11TriggerThreshold_NEL_ID,11)
    ((int)DGCh12TriggerThreshold_NEL_ID,12) ((int)DGCh13TriggerThreshold_NEL_ID,13) ((int)DGCh14TriggerThreshold_NEL_ID,14)
    ((int)DGCh15TriggerThreshold_NEL_ID,15);

  DGChBaselineCalcMin_NEL_ID_Vec += 
    (int)DGCh0BaselineCalcMin_NEL_ID,  (int)DGCh1BaselineCalcMin_NEL_ID,  (int)DGCh2BaselineCalcMin_NEL_ID, 
    (int)DGCh3BaselineCalcMin_NEL_ID,  (int)DGCh4BaselineCalcMin_NEL_ID,  (int)DGCh5BaselineCalcMin_NEL_ID,
    (int)DGCh6BaselineCalcMin_NEL_ID,  (int)DGCh7BaselineCalcMin_NEL_ID,  (int)DGCh8BaselineCalcMin_NEL_ID,
    (int)DGCh9BaselineCalcMin_NEL_ID,  (int)DGCh10BaselineCalcMin_NEL_ID, (int)DGCh11BaselineCalcMin_NEL_ID,
    (int)DGCh12BaselineCalcMin_NEL_ID, (int)DGCh13BaselineCalcMin_NEL_ID, (int)DGCh14BaselineCalcMin_NEL_ID,
    (int)DGCh15BaselineCalcMin_NEL_ID;

  DGChBaselineCalcMax_NEL_ID_Vec += 
    (int)DGCh0BaselineCalcMax_NEL_ID,  (int)DGCh1BaselineCalcMax_NEL_ID,  (int)DGCh2BaselineCalcMax_NEL_ID,
    (int)DGCh3BaselineCalcMax_NEL_ID,  (int)DGCh4BaselineCalcMax_NEL_ID,  (int)DGCh5BaselineCalcMax_NEL_ID, 
    (int)DGCh6BaselineCalcMax_NEL_ID,  (int)DGCh7BaselineCalcMax_NEL_ID,  (int)DGCh7BaselineCalcMax_NEL_ID,
    (int)DGCh9BaselineCalcMax_NEL_ID,  (int)DGCh10BaselineCalcMax_NEL_ID, (int)DGCh11BaselineCalcMax_NEL_ID,
    (int)DGCh12BaselineCalcMax_NEL_ID, (int)DGCh13BaselineCalcMax_NEL_ID, (int)DGCh14BaselineCalcMax_NEL_ID,
    (int)DGCh15BaselineCalcMax_NEL_ID;

  
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
  for(int ch=0; ch<NumDGChannels; ch++){

    // Each channel's widgets are grouped under its own TGGroupFrame.
    TGGroupFrame *DGChannelControl_GF = new TGGroupFrame(DGChannelControls_VF, DGChannelLabels[ch].c_str(), kVerticalFrame);
    DGChannelControls_VF->AddFrame(DGChannelControl_GF, new TGLayoutHints(kLHintsCenterX, 5, 5, 5, 0));
    DGChannelControl_GF->SetTitlePos(TGGroupFrame::kLeft);
    

    ////////////////////////////////////////////////////
    // CAEN STD firmware GUI channel-specific widgets //
    ////////////////////////////////////////////////////

    if(DGStandardFW_RB->IsDown()){

      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "Acquisition settings"),
				    new TGLayoutHints(kLHintsLeft,0,0,5,0));
    
      // Horizontal frame to hold the "enable" and "pulse polarity" buttons
      TGHorizontalFrame *DGChannelControl_HF = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(DGChannelControl_HF);
    
      // ROOT check button to enable channel for digitization
      DGChannelControl_HF->AddFrame(DGChEnable_CB[ch] = new TGCheckButton(DGChannelControl_HF, "Enable", DGChEnable_CB_ID_Vec[ch]),
				    new TGLayoutHints(kLHintsCenterY,10,0,0,0));
      if(ch == 0) 
	DGChEnable_CB[ch]->SetState(kButtonDown);

      // TGLabel for the pulse polarity radio buttons

      DGChannelControl_HF->AddFrame(new TGLabel(DGChannelControl_HF,"Polarity:"),
				    new TGLayoutHints(kLHintsCenterY,25,0,5,0));

      TGHButtonGroup *DGChPolarity_BG = new TGHButtonGroup(DGChannelControl_HF, "");
      DGChPolarity_BG->SetTitlePos(TGButtonGroup::kCenter);
      DGChPolarity_BG->SetBorderDrawn(false);
      DGChannelControl_HF->AddFrame(DGChPolarity_BG, new TGLayoutHints(kLHintsNormal,-2,-15,-10,-10));

      DGChPosPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "+  ", -1);
      DGChNegPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "-", -1);
      DGChNegPolarity_RB[ch]->SetState(kButtonDown);
      DGChPolarity_BG->Show();

      // ADAQ number entry to set channel's DAC offset [hex : 0x0000 - 0xffff]]
      DGChannelControl_GF->AddFrame(DGChDCOffset_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "DC offset (hex)", DGChDCOffset_NEL_ID_Vec[ch]),
				    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESHex);
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
      DGChDCOffset_NEL[ch]->GetEntry()->SetLimitValues(0x0000,0xffff);
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumber(0x8000);
      DGChDCOffset_NEL[ch]->GetEntry()->Resize(55,20);
      
      // ADAQ number entry to set channel's trigger threshold [ADC]
      DGChannelControl_GF->AddFrame(DGChTriggerThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Trigger threshold (ADC)", DGChTriggerThreshold_NEL_ID_Vec[ch]),
				    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
      DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChTriggerThreshold_NEL[ch]->GetEntry()->Resize(55,20);

      Int_t BitDepth = AAVMEManager::GetInstance()->GetDGManager()->GetNumADCBits();
      Int_t Trigger = pow(2,(BitDepth-1));
      DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(Trigger);

    
      ////////////////////////////////////////
      // Channel-specific analysis settings //
      ////////////////////////////////////////

      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "Baseline analysis (abs. sample)"),
				    new TGLayoutHints(kLHintsLeft,0,0,10,5));
    
      TGHorizontalFrame *DGBaseline_HF = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(DGBaseline_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
    
      // ADAQ number entry to set minimum sample for baseline calculation [sample]
      DGBaseline_HF->AddFrame(DGChBaselineCalcMin_NEL[ch] = new ADAQNumberEntryWithLabel(DGBaseline_HF, "Min.", DGChBaselineCalcMin_NEL_ID_Vec[ch]),
			      new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChBaselineCalcMin_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
    
      DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetNumber(0);
      DGChBaselineCalcMin_NEL[ch]->GetEntry()->Resize(55,20);

      // ADAQ number entry to set maximum sample for baseline calculation [sample]
      DGBaseline_HF->AddFrame(DGChBaselineCalcMax_NEL[ch] = new ADAQNumberEntryWithLabel(DGBaseline_HF, "Max.", DGChBaselineCalcMax_NEL_ID_Vec[ch]),
			      new TGLayoutHints(kLHintsNormal, 15,0.0,0));
      DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetNumber(30);
      DGChBaselineCalcMax_NEL[ch]->GetEntry()->Resize(55,20);


      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "PSD analysis (sample rel. to peak)"),
				    new TGLayoutHints(kLHintsLeft,0,0,10,5));
    
      TGHorizontalFrame *DGPSDTotal_HF = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(DGPSDTotal_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

      DGPSDTotal_HF->AddFrame(new TGLabel(DGPSDTotal_HF, "Total: "),
			      new TGLayoutHints(kLHintsLeft,10,0,5,0));
    
      DGPSDTotal_HF->AddFrame(DGChPSDTotalStart_NEL[ch] = new ADAQNumberEntryWithLabel(DGPSDTotal_HF,
										       "Start",
										       -1),
			      new TGLayoutHints(kLHintsNormal, 0,10,0,0));
      DGChPSDTotalStart_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPSDTotalStart_NEL[ch]->GetEntry()->SetNumber(-10);
      DGChPSDTotalStart_NEL[ch]->GetEntry()->Resize(45,20);
    
    
      DGPSDTotal_HF->AddFrame(DGChPSDTotalStop_NEL[ch] = new ADAQNumberEntryWithLabel(DGPSDTotal_HF,
										      "Stop",
										      -1),
			      new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      DGChPSDTotalStop_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPSDTotalStop_NEL[ch]->GetEntry()->SetNumber(39);
      DGChPSDTotalStop_NEL[ch]->GetEntry()->Resize(45,20);


      TGHorizontalFrame *DGPSDTail_HF = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(DGPSDTail_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
    
      DGPSDTail_HF->AddFrame(new TGLabel(DGPSDTail_HF, "Tail:  "),
			     new TGLayoutHints(kLHintsLeft,10,0,5,0));
   
      DGPSDTail_HF->AddFrame(DGChPSDTailStart_NEL[ch] = new ADAQNumberEntryWithLabel(DGPSDTail_HF,
										     "Start",
										     -1),
			     new TGLayoutHints(kLHintsNormal, 0,10,0,0));
      DGChPSDTailStart_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPSDTailStart_NEL[ch]->GetEntry()->SetNumber(7);
      DGChPSDTailStart_NEL[ch]->GetEntry()->Resize(45,20);
    
    
      DGPSDTail_HF->AddFrame(DGChPSDTailStop_NEL[ch] = new ADAQNumberEntryWithLabel(DGPSDTail_HF,
										    "Stop",
										    -1),
			     new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      DGChPSDTailStop_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPSDTailStop_NEL[ch]->GetEntry()->SetNumber(39);
      DGChPSDTailStop_NEL[ch]->GetEntry()->Resize(45,20);

            DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "Zero Length Encoding (ZLE)"),
				    new TGLayoutHints(kLHintsLeft,0,0,10,0));
    
      TGHorizontalFrame *ZLE_HF1 = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(ZLE_HF1, new TGLayoutHints(kLHintsNormal, 10,0,0,0));

      ZLE_HF1->AddFrame(new TGLabel(ZLE_HF1, "Logic: "),
			new TGLayoutHints(kLHintsNormal, 0,0,5,0));
    
      TGHButtonGroup *ZLELogicButtons_BG = new TGHButtonGroup(ZLE_HF1,"");
      ZLELogicButtons_BG->SetBorderDrawn(false);
      ZLE_HF1->AddFrame(ZLELogicButtons_BG, new TGLayoutHints(kLHintsNormal, -1,-15,-10,-10));
    
      DGChZLEPosLogic_RB[ch] = new TGRadioButton(ZLELogicButtons_BG, "+  ", -1);
    
      DGChZLENegLogic_RB[ch] = new TGRadioButton(ZLELogicButtons_BG, "-", -1);
      DGChZLENegLogic_RB[ch]->SetState(kButtonDown);

    
      DGChannelControl_GF->AddFrame(DGChZLEThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Threshold (ADC)", -1),
				    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChZLEThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChZLEThreshold_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
      DGChZLEThreshold_NEL[ch]->GetEntry()->SetNumber(2100);
      DGChZLEThreshold_NEL[ch]->GetEntry()->Resize(55,20);
    

      TGHorizontalFrame *ZLE_HF0 = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(ZLE_HF0, new TGLayoutHints(kLHintsNormal, 10,0,0,0));
    
      ZLE_HF0->AddFrame(DGChZLEForward_NEL[ch] = new ADAQNumberEntryWithLabel(ZLE_HF0, "Frwd", -1),
			new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      DGChZLEForward_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChZLEForward_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChZLEForward_NEL[ch]->GetEntry()->SetNumber(10);
      DGChZLEForward_NEL[ch]->GetEntry()->Resize(55,20);

      ZLE_HF0->AddFrame(DGChZLEBackward_NEL[ch] = new ADAQNumberEntryWithLabel(ZLE_HF0, "Back", -1),
			new TGLayoutHints(kLHintsNormal, 15,0,0,0));
      DGChZLEBackward_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChZLEBackward_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChZLEBackward_NEL[ch]->GetEntry()->SetNumber(10);
      DGChZLEBackward_NEL[ch]->GetEntry()->Resize(55,20);
    }
    else if(DGPSDFW_RB->IsDown()){


      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "Acquisition settings"),
				    new TGLayoutHints(kLHintsLeft,0,0,5,0));
    
      // Horizontal frame to hold the "enable" and "pulse polarity" buttons
      TGHorizontalFrame *DGChannelControl_HF = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(DGChannelControl_HF);
    
      // ROOT check button to enable channel for digitization
      DGChannelControl_HF->AddFrame(DGChEnable_CB[ch] = new TGCheckButton(DGChannelControl_HF, "Enable", DGChEnable_CB_ID_Vec[ch]),
				    new TGLayoutHints(kLHintsCenterY,10,0,0,0));
      if(ch == 0) 
	DGChEnable_CB[ch]->SetState(kButtonDown);

      // TGLabel for the pulse polarity radio buttons

      DGChannelControl_HF->AddFrame(new TGLabel(DGChannelControl_HF,"Polarity:"),
				    new TGLayoutHints(kLHintsCenterY,25,0,5,0));

      TGHButtonGroup *DGChPolarity_BG = new TGHButtonGroup(DGChannelControl_HF, "");
      DGChPolarity_BG->SetTitlePos(TGButtonGroup::kCenter);
      DGChPolarity_BG->SetBorderDrawn(false);
      DGChannelControl_HF->AddFrame(DGChPolarity_BG, new TGLayoutHints(kLHintsNormal,-2,-15,-10,-10));

      DGChPosPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "+  ", -1);
      DGChNegPolarity_RB[ch] = new TGRadioButton(DGChPolarity_BG, "-", -1);
      DGChNegPolarity_RB[ch]->SetState(kButtonDown);
      DGChPolarity_BG->Show();

      // ADAQ number entry to set channel's DAC offset [hex : 0x0000 - 0xffff]]
      DGChannelControl_GF->AddFrame(DGChDCOffset_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "DC offset (hex)", DGChDCOffset_NEL_ID_Vec[ch]),
				    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESHex);
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
      DGChDCOffset_NEL[ch]->GetEntry()->SetLimitValues(0x0000,0xffff);
      DGChDCOffset_NEL[ch]->GetEntry()->SetNumber(0x8000);
      DGChDCOffset_NEL[ch]->GetEntry()->Resize(55,20);
      
      DGChannelControl_GF->AddFrame(DGChRecordLength_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF,
											    "Record length (samples)",
											    -1),
				    new TGLayoutHints(kLHintsLeft,10,0,0,0));
      DGChRecordLength_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChRecordLength_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChRecordLength_NEL[ch]->GetEntry()->Resize(55,20);
      DGChRecordLength_NEL[ch]->GetEntry()->SetNumber(512);

      DGChannelControl_GF->AddFrame(DGChBaselineSamples_CBL[ch] = new ADAQComboBoxWithLabel(DGChannelControl_GF, "Baseline (samples)", -1),
				    new TGLayoutHints(kLHintsLeft, 10,0,0,0));
      DGChBaselineSamples_CBL[ch]->GetComboBox()->AddEntry("8",2);
      DGChBaselineSamples_CBL[ch]->GetComboBox()->AddEntry("32",4);
      DGChBaselineSamples_CBL[ch]->GetComboBox()->AddEntry("128",6);
      DGChBaselineSamples_CBL[ch]->GetComboBox()->Resize(57,20);
      DGChBaselineSamples_CBL[ch]->GetComboBox()->Select(4);

      DGChannelControl_GF->AddFrame(DGChChargeSensitivity_CBL[ch] = new ADAQComboBoxWithLabel(DGChannelControl_GF, "Q sensitivity (fC/LSB)", -1),
				    new TGLayoutHints(kLHintsLeft, 10,0,0,0));
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->AddEntry("40",0);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->AddEntry("160",1);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->AddEntry("640",2);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->AddEntry("2500",3);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->Resize(57,20);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->Select(0);

      DGChannelControl_GF->AddFrame(DGChPSDCut_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF,
										      "PSD readout cut",
										      -1),
				    new TGLayoutHints(kLHintsLeft,10,0,0,0));
      DGChPSDCut_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPSDCut_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChPSDCut_NEL[ch]->GetEntry()->Resize(55,20);
      DGChPSDCut_NEL[ch]->GetEntry()->SetNumber(200);
      DGChPSDCut_NEL[ch]->GetEntry()->SetState(false);

      
      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "Trigger settings"),
				    new TGLayoutHints(kLHintsLeft,0,0,10,5));
      
      TGHorizontalFrame *Trigger_HF0 = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(Trigger_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      
      // ADAQ number entry to set channel's trigger threshold [ADC]
      Trigger_HF0->AddFrame(DGChTriggerThreshold_NEL[ch] = new ADAQNumberEntryWithLabel(Trigger_HF0, "Level (ADC)", DGChTriggerThreshold_NEL_ID_Vec[ch]),
			    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChTriggerThreshold_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
      DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChTriggerThreshold_NEL[ch]->GetEntry()->SetNumber(500);
      DGChTriggerThreshold_NEL[ch]->GetEntry()->Resize(55,20);
      
      Trigger_HF0->AddFrame(DGChTriggerConfig_CBL[ch] = new ADAQComboBoxWithLabel(Trigger_HF0, "", -1),
			    new TGLayoutHints(kLHintsLeft, 5,0,0,0));
      DGChTriggerConfig_CBL[ch]->GetComboBox()->AddEntry("Peak",0);
      DGChTriggerConfig_CBL[ch]->GetComboBox()->AddEntry("Threshold",1);
      DGChTriggerConfig_CBL[ch]->GetComboBox()->Resize(80,20);
      DGChTriggerConfig_CBL[ch]->GetComboBox()->Select(1);

      /* Trigger holdoff should be channel-specific but it is NOT as
	 of CAENDigitizer-2.6.7. I have asked CAEN for
	 clarification. At present, the trigger holdoff settings is
	 handled as a global parameters below in the 'Trigger Control'
	 section. ZSH (28 Sep 15)

	 DGChannelControl_GF->AddFrame(DGChTriggerHoldoff_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Holdoff (samples)", -1),
                                       new TGLayoutHints(kLHintsNormal, 10,0,0,0));
	 DGChTriggerHoldoff_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
	 DGChTriggerHoldoff_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
	 DGChTriggerHoldoff_NEL[ch]->GetEntry()->SetNumber(50);
	 DGChTriggerHoldoff_NEL[ch]->GetEntry()->Resize(55,20);
      */

      DGChannelControl_GF->AddFrame(DGChTriggerValidation_NEL[ch] = new ADAQNumberEntryWithLabel(DGChannelControl_GF, "Validation (samples)", -1),
			       new TGLayoutHints(kLHintsNormal, 10,0,0,0));
      DGChTriggerValidation_NEL[ch]->GetEntry()->Connect("ValueSet(Long_t)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
      DGChTriggerValidation_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChTriggerValidation_NEL[ch]->GetEntry()->SetNumber(50);
      DGChTriggerValidation_NEL[ch]->GetEntry()->Resize(55,20);
      
      // PSD integral (gate) settings

      DGChannelControl_GF->AddFrame(new TGLabel(DGChannelControl_GF, "PSD integral settings (samples)"),
				    new TGLayoutHints(kLHintsLeft,0,0,10,5));

      TGHorizontalFrame *PSD_HF0 = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(PSD_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
      
      PSD_HF0->AddFrame(DGChShortGate_NEL[ch] = new ADAQNumberEntryWithLabel(PSD_HF0,
									     "Short  ",
									     -1),
			new TGLayoutHints(kLHintsLeft,10,0,0,0));
      DGChShortGate_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChShortGate_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChShortGate_NEL[ch]->GetEntry()->Resize(49,20);
      DGChShortGate_NEL[ch]->GetEntry()->SetNumber(100);
      
      PSD_HF0->AddFrame(DGChLongGate_NEL[ch] = new ADAQNumberEntryWithLabel(PSD_HF0,
									    "Long",
									    -1),
			new TGLayoutHints(kLHintsLeft,5,0,0,0));
      DGChLongGate_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChLongGate_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChLongGate_NEL[ch]->GetEntry()->Resize(49,20);
      DGChLongGate_NEL[ch]->GetEntry()->SetNumber(450);

      TGHorizontalFrame *PSD_HF1 = new TGHorizontalFrame(DGChannelControl_GF);
      DGChannelControl_GF->AddFrame(PSD_HF1, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

      PSD_HF1->AddFrame(DGChPreTrigger_NEL[ch] = new ADAQNumberEntryWithLabel(PSD_HF1,
									      "PreTrig",
									      -1),
			new TGLayoutHints(kLHintsLeft,10,0,0,0));
      DGChPreTrigger_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChPreTrigger_NEL[ch]->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
      DGChPreTrigger_NEL[ch]->GetEntry()->Resize(49,20);
      DGChPreTrigger_NEL[ch]->GetEntry()->SetNumber(100);
      
      PSD_HF1->AddFrame(DGChGateOffset_NEL[ch] = new ADAQNumberEntryWithLabel(PSD_HF1,
									      "Gate offset",
									      -1),
			new TGLayoutHints(kLHintsLeft,5,0,0,0));
      DGChGateOffset_NEL[ch]->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
      DGChGateOffset_NEL[ch]->GetEntry()->Resize(49,20);
      DGChGateOffset_NEL[ch]->GetEntry()->SetNumber(50);
    }
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
		       new TGLayoutHints(kLHintsNormal, 25, 0, 0, 5));
  DisplayHorizontalScale_THS->SetRange(0,1);
  DisplayHorizontalScale_THS->SetPosition(0,1);
  DisplayHorizontalScale_THS->SetPointerPosition(0.5);
  DisplayHorizontalScale_THS->SetBackgroundColor(ColorManager->Number2Pixel(18));
  DisplayHorizontalScale_THS->Connect("PositionChanged()", "AADisplaySlots", DisplaySlots, "HandleDoubleSliders()");
  DisplayHorizontalScale_THS->Connect("PointerPositionChanged()", "AADisplaySlots", DisplaySlots, "HandleSliderPointers()");

  TGHorizontalFrame *DGScopeDisplayButtons_HF = new TGHorizontalFrame(Display_VF);
  DGScopeDisplayButtons_HF->SetBackgroundColor(ColorManager->Number2Pixel(22));
  Display_VF->AddFrame(DGScopeDisplayButtons_HF, new TGLayoutHints(kLHintsNormal, 20,0,5,0));

  // ROOT text button for starting/stopping data acquisition by the digitizer
  DGScopeDisplayButtons_HF->AddFrame(AQStartStop_TB = new TGTextButton(DGScopeDisplayButtons_HF, "Stopped", AQStartStop_TB_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,0,0));
  AQStartStop_TB->Connect("Clicked()", "AADisplaySlots", DisplaySlots, "HandleTextButtons()");
  AQStartStop_TB->Resize(300,30);
  AQStartStop_TB->ChangeOptions(AQStartStop_TB->GetOptions() | kFixedSize);
  AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
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

  TGCompositeFrame *AcquisitionSubtab = AQControlSubtabs->AddTab(" Data acquisition ");
  TGCompositeFrame *AcquisitionSubframe = new TGCompositeFrame(AcquisitionSubtab, 0, 0, kHorizontalFrame);
  AcquisitionSubtab->AddFrame(AcquisitionSubframe);
  
  TGCompositeFrame *SpectrumSubtab = AQControlSubtabs->AddTab(" Pulse spectra ");
  TGCompositeFrame *SpectrumSubframe = new TGCompositeFrame(SpectrumSubtab, 0, 0, kHorizontalFrame);
  SpectrumSubtab->AddFrame(SpectrumSubframe);
  
  TGCompositeFrame *PSDSubtab = AQControlSubtabs->AddTab(" Pulse discrimination ");
  TGCompositeFrame *PSDSubframe = new TGCompositeFrame(PSDSubtab, 0, 0, kHorizontalFrame);
  PSDSubtab->AddFrame(PSDSubframe);
  
  TGCompositeFrame *DataSubtab = AQControlSubtabs->AddTab(" Persistent storage ");
  TGCompositeFrame *DataSubframe = new TGCompositeFrame(DataSubtab, 0, 0, kHorizontalFrame);
  DataSubtab->AddFrame(DataSubframe);
  
  TGCompositeFrame *GraphicsSubtab = AQControlSubtabs->AddTab(" Graphics display ");
  TGCompositeFrame *GraphicsSubframe = new TGCompositeFrame(GraphicsSubtab, 0, 0, kHorizontalFrame);
  GraphicsSubtab->AddFrame(GraphicsSubframe);

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
  TGButtonGroup *DGScopeMode_BG = new TGButtonGroup(DGScopeModeAndTrigger_VF, "Acquisition display", kVerticalFrame);
  DGScopeMode_BG->SetTitlePos(TGButtonGroup::kCenter);
  DGScopeModeAndTrigger_VF->AddFrame(DGScopeMode_BG, new TGLayoutHints(kLHintsExpandX,5,5,5,5));
  
  AQWaveform_RB = new TGRadioButton(DGScopeMode_BG, "Digitized waveform", AQWaveform_RB_ID);
  AQWaveform_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  AQWaveform_RB->SetState(kButtonDown);
  
  AQSpectrum_RB = new TGRadioButton(DGScopeMode_BG, "Pulse spectrum", AQSpectrum_RB_ID);
  AQSpectrum_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  AQPSDHistogram_RB = new TGRadioButton(DGScopeMode_BG, "PSD Histogram", AQPSDHistogram_RB_ID);
  AQPSDHistogram_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  DGScopeMode_BG->Show();


  ///////////////////
  // Trigger controls
  
  TGGroupFrame *DGTriggerControls_GF = new TGGroupFrame(DGScopeModeAndTrigger_VF, "Trigger control", kVerticalFrame);
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
  if(DGStandardFW_RB->IsDown()){
    DGTriggerControls_GF->AddFrame(DGTriggerEdge_CBL = new ADAQComboBoxWithLabel(DGTriggerControls_GF, "Edge", DGTriggerEdge_CBL_ID),
				   new TGLayoutHints(kLHintsNormal,5,5,0,5));
    DGTriggerEdge_CBL->GetComboBox()->AddEntry("Rising",0);
    DGTriggerEdge_CBL->GetComboBox()->AddEntry("Falling",1);
    DGTriggerEdge_CBL->GetComboBox()->Select(1);
    DGTriggerEdge_CBL->GetComboBox()->Resize(110,20);
    DGTriggerEdge_CBL->GetComboBox()->ChangeOptions(DGTriggerEdge_CBL->GetComboBox()->GetOptions() | kFixedSize);
  }
  else if(DGPSDFW_RB->IsDown()){
    DGTriggerControls_GF->AddFrame(DGPSDTriggerHoldoff_NEL = new ADAQNumberEntryWithLabel(DGTriggerControls_GF, "Holdoff (samples)", DGPSDTriggerHoldoff_NEL_ID),
				   new TGLayoutHints(kLHintsNormal,5,0,0,5));
    DGPSDTriggerHoldoff_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGPSDTriggerHoldoff_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    DGPSDTriggerHoldoff_NEL->GetEntry()->Resize(60,20);
    DGPSDTriggerHoldoff_NEL->GetEntry()->SetNumber(10);
  }
  
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

  TGGroupFrame *DGAcquisitionControl_GF = new TGGroupFrame(AcquisitionSubframe, "Acquisition", kVerticalFrame);
  DGAcquisitionControl_GF->SetTitlePos(TGGroupFrame::kCenter);
  AcquisitionSubframe->AddFrame(DGAcquisitionControl_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));
  
  DGAcquisitionControl_GF->AddFrame(DGAcquisitionControl_CBL = new ADAQComboBoxWithLabel(DGAcquisitionControl_GF, 
											 "Control style",
											 DGAcquisitionControl_CBL_ID),
				     new TGLayoutHints(kLHintsNormal,5,5,10,5));
  DGAcquisitionControl_CBL->GetComboBox()->AddEntry("Standard",0);
  DGAcquisitionControl_CBL->GetComboBox()->AddEntry("Gated (NIM)",1);
  DGAcquisitionControl_CBL->GetComboBox()->AddEntry("Gated (TTL)",2);
  DGAcquisitionControl_CBL->GetComboBox()->Select(0);
  
  if(DGStandardFW_RB->IsDown()){
    
    // ADAQ number entry specifying number of samples
    DGAcquisitionControl_GF->AddFrame(DGRecordLength_NEL = new ADAQNumberEntryWithLabel(DGAcquisitionControl_GF, "Record length (#)", DGRecordLength_NEL_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,0));
    DGRecordLength_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGRecordLength_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
    DGRecordLength_NEL->GetEntry()->SetNumber(500);
    
    // ADAQ number entry specifying the percentage of the acquisition
    // window that is behind (or after) the triggern (all channels)
    DGAcquisitionControl_GF->AddFrame(DGPostTrigger_NEL = new ADAQNumberEntryWithLabel(DGAcquisitionControl_GF, "Post trigger (%)", DGPostTriggerSize_NEL_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,0,5));
    DGPostTrigger_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
    DGPostTrigger_NEL->GetEntry()->SetNumLimits(TGNumberFormat::kNELLimitMinMax);
    DGPostTrigger_NEL->GetEntry()->SetLimitValues(0,100);
    DGPostTrigger_NEL->GetEntry()->SetNumber(70);
  }
  else if(DGPSDFW_RB->IsDown()){

    // The DPP-PSD "Oscilloscope" mode - called "Waveform only" mode
    // here - has been discontinued for x725 and x730 but will remain
    // implemented on x720/x790 digitizers. Enable user to use this
    // mode only for appropriate boards

    ZBoardType DGType = TheVMEManager->GetDGManager()->GetBoardType();
    Bool_t EnableOscilloscopeMode = false;
    if(DGType == zV1720 or DGType == zDT5720 or
       DGType == zDT5790M or DGType == zDT5790N or DGType == zDT5790P)
      EnableOscilloscopeMode = true;
    
    DGAcquisitionControl_GF->AddFrame(DGPSDMode_CBL = new ADAQComboBoxWithLabel(DGAcquisitionControl_GF, "PSD mode", DGPSDMode_CBL_ID),
				      new TGLayoutHints(kLHintsNormal,5,5,5,0));
    DGPSDMode_CBL->GetComboBox()->Resize(130, 20);
    if(EnableOscilloscopeMode)
      DGPSDMode_CBL->GetComboBox()->AddEntry("Waveform only", 0);
    DGPSDMode_CBL->GetComboBox()->AddEntry("List only", 1);
    DGPSDMode_CBL->GetComboBox()->AddEntry("List + Waveform", 2);
    DGPSDMode_CBL->GetComboBox()->Select(2);
    DGPSDMode_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");

    // Set which acquired DPP-PSD data is used in analysis (spectra
    // and PSD histograms): the list data or the full waveform

    TGHorizontalFrame *DGPSDAnalysis_HF = new TGHorizontalFrame(DGAcquisitionControl_GF);
    DGAcquisitionControl_GF->AddFrame(DGPSDAnalysis_HF,
				      new TGLayoutHints(kLHintsNormal,0,0,0,0));

    DGPSDAnalysis_HF->AddFrame(new TGLabel(DGPSDAnalysis_HF,"Data analysis: "),
			       new TGLayoutHints(kLHintsNormal,5,0,5,5));

    DGPSDAnalysis_HF->AddFrame(DGPSDListAnalysis_RB = new TGRadioButton(DGPSDAnalysis_HF, "List", DGPSDListAnalysis_RB_ID),
			       new TGLayoutHints(kLHintsNormal,5,0,5,5));
    DGPSDListAnalysis_RB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
    //DGPSDListAnalysis_RB->SetState(kButtonDown);
    
    DGPSDAnalysis_HF->AddFrame(DGPSDWaveformAnalysis_RB = new TGRadioButton(DGPSDAnalysis_HF, "Waveform", DGPSDWaveformAnalysis_RB_ID),
			       new TGLayoutHints(kLHintsNormal,5,0,5,5));
    DGPSDWaveformAnalysis_RB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
    DGPSDWaveformAnalysis_RB->SetState(kButtonDown);
  }
  
  DGAcquisitionControl_GF->AddFrame(AQTime_NEL = new ADAQNumberEntryWithLabel(DGAcquisitionControl_GF, "Acquisition time (s)", AQTime_NEL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  AQTime_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  AQTime_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  AQTime_NEL->GetEntry()->SetNumber(10);

  DGAcquisitionControl_GF->AddFrame(AQTimer_NEFL = new ADAQNumberEntryFieldWithLabel(DGAcquisitionControl_GF, "Countdown", AQTimer_NEFL_ID),
					  new TGLayoutHints(kLHintsNormal,5,5,0,5));
  AQTimer_NEFL->GetEntry()->SetFormat(TGNumberFormat::kNESMinSec);
  AQTimer_NEFL->GetEntry()->SetNumber(10);
  AQTimer_NEFL->GetEntry()->SetState(false);

  TGHorizontalFrame *DGScopeTimerButtons_HF = new TGHorizontalFrame(DGAcquisitionControl_GF);
  DGAcquisitionControl_GF->AddFrame(DGScopeTimerButtons_HF);
  
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
  DGEventsBeforeReadout_NEL->GetEntry()->SetNumber(25);
  
  DGScopeReadoutControls_GF->AddFrame(DGCheckBufferStatus_TB = new TGTextButton(DGScopeReadoutControls_GF, "Check FPGA Buffer", CheckBufferStatus_TB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  DGCheckBufferStatus_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  DGCheckBufferStatus_TB->Resize(150,30);
  DGCheckBufferStatus_TB->ChangeOptions(DGCheckBufferStatus_TB->GetOptions() | kFixedSize);
  
  DGScopeReadoutControls_GF->AddFrame(DGBufferStatus_PB = new TGHProgressBar(DGScopeReadoutControls_GF, TGProgressBar::kFancy, 200),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  DGBufferStatus_PB->SetHeight(20);
  DGBufferStatus_PB->SetBarColor(ColorManager->Number2Pixel(kGreen-5));
  DGBufferStatus_PB->ShowPosition(kTRUE, kFALSE, "%.2f% buffer capacity");

  DGScopeReadoutControls_GF->AddFrame(AQDataReductionEnable_CB = new TGCheckButton(DGScopeReadoutControls_GF, "Enable data reduction", AQDataReductionEnable_CB_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,5,0));

  DGScopeReadoutControls_GF->AddFrame(AQDataReductionFactor_NEL = new ADAQNumberEntryWithLabel(DGScopeReadoutControls_GF, "Data reduction factor", AQDataReductionFactor_NEL_ID),
				      new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  AQDataReductionFactor_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  AQDataReductionFactor_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  AQDataReductionFactor_NEL->GetEntry()->SetNumber(1);

  DGScopeReadoutControls_GF->AddFrame(DGZLEEnable_CB = new TGCheckButton(DGScopeReadoutControls_GF, "Enable ZLE zero-suppression", DGZLEEnable_CB_ID),
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
  for(uint32_t ch=0; ch<NumDGChannels; ch++)
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
  SpectrumMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  SpectrumMaxBin_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumMaxBin_NEL->GetEntry()->SetNumber(30000.);
  
  TGHorizontalFrame *SpectrumAxis_HF = new TGHorizontalFrame(SpectrumHistogram_GF);
  SpectrumHistogram_GF->AddFrame(SpectrumAxis_HF, new TGLayoutHints(kLHintsNormal,0,0,0,0));


  /////////////////
  // Pulse analysis

  TGGroupFrame *SpectrumAnalysis_GF = new TGGroupFrame(SpectrumSubframe,"Analysis",kVerticalFrame);
  SpectrumAnalysis_GF->SetTitlePos(TGGroupFrame::kCenter);
  SpectrumSubframe->AddFrame(SpectrumAnalysis_GF);

  TGVButtonGroup *SpectrumAnalysis_BG = new TGVButtonGroup(SpectrumAnalysis_GF,"Analysis");
  SpectrumAnalysis_BG->SetBorderDrawn(false);
  SpectrumAnalysis_GF->AddFrame(SpectrumAnalysis_BG, new TGLayoutHints(kLHintsNormal,-13,0,0,-3));
  
  SpectrumPulseHeight_RB = new TGRadioButton(SpectrumAnalysis_BG, "Pulse height spectrum  ", SpectrumPulseHeight_RB_ID);
  SpectrumPulseHeight_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  
  SpectrumPulseArea_RB = new TGRadioButton(SpectrumAnalysis_BG, "Pulse area spectrum", SpectrumPulseArea_RB_ID);
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
  
  for(uint32_t ch=0; ch<NumDGChannels; ch++)
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

  TGHorizontalFrame *SpectrumCalibration_HF5 = new TGHorizontalFrame(SpectrumCalibration_GF);
  SpectrumCalibration_GF->AddFrame(SpectrumCalibration_HF5, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  SpectrumCalibration_HF5->AddFrame(SpectrumCalibrationEnergy_NEL = new ADAQNumberEntryWithLabel(SpectrumCalibration_HF5, "Energy", SpectrumCalibrationEnergy_NEL_ID),
				    new TGLayoutHints(kLHintsLeft,0,0,0,0));
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEANonNegative);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
  SpectrumCalibrationEnergy_NEL->GetEntry()->SetState(false);
  SpectrumCalibrationEnergy_NEL->GetEntry()->Connect("ValueSet(long)", "AASubtabSlots", SubtabSlots, "HandleNumberEntries()");
  
  SpectrumCalibration_HF5->AddFrame(SpectrumCalibrationUnit_CBL = new ADAQComboBoxWithLabel(SpectrumCalibration_HF5, "", SpectrumCalibrationUnit_CBL_ID),
				    new TGLayoutHints(kLHintsNormal, 10,0,0,0));
  SpectrumCalibrationUnit_CBL->GetComboBox()->Resize(50,20);
  SpectrumCalibrationUnit_CBL->GetComboBox()->AddEntry("keV",0);
  SpectrumCalibrationUnit_CBL->GetComboBox()->AddEntry("MeV",1);
  SpectrumCalibrationUnit_CBL->GetComboBox()->Select(1);
  SpectrumCalibrationUnit_CBL->GetComboBox()->SetEnabled(false);
  SpectrumCalibrationUnit_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");

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


  //////////////////////////
  // Pulse discrimination //
  //////////////////////////
  
  TGGroupFrame *PSDHistogram_GF = new TGGroupFrame(PSDSubframe, "PSD histogram", kVerticalFrame);
  PSDHistogram_GF->SetTitlePos(TGGroupFrame::kCenter);
  PSDSubframe->AddFrame(PSDHistogram_GF, new TGLayoutHints(kLHintsNormal, 5,5,5,5));

  TGHorizontalFrame *PSDOptions_HF = new TGHorizontalFrame(PSDHistogram_GF);
  PSDHistogram_GF->AddFrame(PSDOptions_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  // ADAQ combo box for selecting the channel for display spectrum
  PSDOptions_HF->AddFrame(PSDChannel_CBL = new ADAQComboBoxWithLabel(PSDOptions_HF, "", PSDChannel_CBL_ID),
			  new TGLayoutHints(kLHintsNormal,0,0,10,0));
  for(uint32_t ch=0; ch<NumDGChannels; ch++)
    PSDChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  PSDChannel_CBL->GetComboBox()->Select(0);
  PSDChannel_CBL->GetComboBox()->Resize(80,20);
  PSDChannel_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");
  
  TGVerticalFrame *PSDOptions_VF = new TGVerticalFrame(PSDOptions_HF);
  PSDOptions_HF->AddFrame(PSDOptions_VF, new TGLayoutHints(kLHintsNormal,0,0,0,0));
  
  PSDOptions_VF->AddFrame(PSDYAxisTail_RB = new TGRadioButton(PSDOptions_VF, "Total v. tail", PSDYAxisTail_RB_ID),
			  new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  PSDYAxisTail_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  PSDYAxisTail_RB->SetState(kButtonDown);
  
  PSDOptions_VF->AddFrame(PSDYAxisTailTotal_RB = new TGRadioButton(PSDOptions_VF, "Total v. tail/total", PSDYAxisTailTotal_RB_ID),
			  new TGLayoutHints(kLHintsNormal, 0,0,0,10));
  PSDYAxisTailTotal_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  

  PSDHistogram_GF->AddFrame(PSDTotalBins_NEL = new ADAQNumberEntryWithLabel(PSDHistogram_GF, "Number of total bins", -1),
			    new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  PSDTotalBins_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  PSDTotalBins_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  PSDTotalBins_NEL->GetEntry()->SetNumber(100);
  PSDTotalBins_NEL->GetEntry()->Resize(60, 20);

  TGHorizontalFrame *PSDTotal_HF = new TGHorizontalFrame(PSDHistogram_GF);
  PSDHistogram_GF->AddFrame(PSDTotal_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  PSDTotal_HF->AddFrame(PSDTotalMinBin_NEL = new ADAQNumberEntryWithLabel(PSDTotal_HF, "Min.", -1),
			new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  PSDTotalMinBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  PSDTotalMinBin_NEL->GetEntry()->SetNumber(0);
  PSDTotalMinBin_NEL->GetEntry()->Resize(60, 20);
  
  PSDTotal_HF->AddFrame(PSDTotalMaxBin_NEL = new ADAQNumberEntryWithLabel(PSDTotal_HF, "Max.", -1),
			new TGLayoutHints(kLHintsNormal, 10,0,0,0));
  PSDTotalMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  PSDTotalMaxBin_NEL->GetEntry()->SetNumber(10000);
  PSDTotalMaxBin_NEL->GetEntry()->Resize(60, 20);

  
  PSDHistogram_GF->AddFrame(PSDTailBins_NEL = new ADAQNumberEntryWithLabel(PSDHistogram_GF, "Number of tail bins", -1),
			    new TGLayoutHints(kLHintsNormal, 0,0,10,0));
  PSDTailBins_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  PSDTailBins_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  PSDTailBins_NEL->GetEntry()->SetNumber(100);
  PSDTailBins_NEL->GetEntry()->Resize(60, 20);

  TGHorizontalFrame *PSDTail_HF = new TGHorizontalFrame(PSDHistogram_GF);
  PSDHistogram_GF->AddFrame(PSDTail_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  PSDTail_HF->AddFrame(PSDTailMinBin_NEL = new ADAQNumberEntryWithLabel(PSDTail_HF, "Min.", -1),
		       new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  PSDTailMinBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  PSDTailMinBin_NEL->GetEntry()->SetNumber(0);
  PSDTailMinBin_NEL->GetEntry()->Resize(60, 20);
  
  PSDTail_HF->AddFrame(PSDTailMaxBin_NEL = new ADAQNumberEntryWithLabel(PSDTail_HF, "Max.", -1),
		       new TGLayoutHints(kLHintsNormal, 10,0,0,0));
  PSDTailMaxBin_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  PSDTailMaxBin_NEL->GetEntry()->SetNumber(3000);
  PSDTailMaxBin_NEL->GetEntry()->Resize(60, 20);
  
  
  PSDHistogram_GF->AddFrame(PSDThreshold_NEL = new ADAQNumberEntryWithLabel(PSDHistogram_GF, "Threshold (ADC)", -1),
			    new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  PSDThreshold_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESReal);
  PSDThreshold_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  PSDThreshold_NEL->GetEntry()->SetNumber(100);
  PSDThreshold_NEL->GetEntry()->Resize(60, 20);
  

  //////////////////
  // Data storage //
  //////////////////

  TGGroupFrame *WaveformStorage_GF = new TGGroupFrame(DataSubframe, "Data storage", kVerticalFrame);
  WaveformStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DataSubframe->AddFrame(WaveformStorage_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));

  WaveformStorage_GF->AddFrame(WaveformFileName_TB = new TGTextButton(WaveformStorage_GF, "ADAQ file name", WaveformFileName_TB_ID),
				  new TGLayoutHints(kLHintsNormal,5,5,5,0));
  WaveformFileName_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformFileName_TB->Resize(220, 30);
  WaveformFileName_TB->ChangeOptions(WaveformFileName_TB->GetOptions() | kFixedSize);

  WaveformStorage_GF->AddFrame(WaveformFileName_TEL = new ADAQTextEntryWithLabel(WaveformStorage_GF, "", -1),
				      new TGLayoutHints(kLHintsNormal,5,0,5,5));
  WaveformFileName_TEL->GetEntry()->Resize(220, 25);
  WaveformFileName_TEL->GetEntry()->ChangeOptions(WaveformFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  WaveformFileName_TEL->GetEntry()->SetState(false);
  WaveformFileName_TEL->GetEntry()->SetText("DefaultData.adaq.root");
  
 
  WaveformStorage_GF->AddFrame(WaveformStoreRaw_CB = new TGCheckButton(WaveformStorage_GF,"Store raw waveforms",WaveformStoreRaw_CB_ID),
			       new TGLayoutHints(kLHintsNormal,5,5,0,0));
  WaveformStoreRaw_CB->SetState(kButtonDown);
  
  WaveformStorage_GF->AddFrame(WaveformStoreEnergyData_CB = new TGCheckButton(WaveformStorage_GF,"Store energy data",WaveformStoreEnergyData_CB_ID),
			       new TGLayoutHints(kLHintsNormal,5,5,0,0));

  WaveformStorage_GF->AddFrame(WaveformStorePSDData_CB = new TGCheckButton(WaveformStorage_GF,"Store PSD data",WaveformStorePSDData_CB_ID),
			       new TGLayoutHints(kLHintsNormal,5,5,0,0));


  TGHorizontalFrame *WaveformCreateClose_HF = new TGHorizontalFrame(WaveformStorage_GF);
  WaveformStorage_GF->AddFrame(WaveformCreateClose_HF, new TGLayoutHints(kLHintsNormal,0,0,5,0));
  
  WaveformCreateClose_HF->AddFrame(WaveformCreateFile_TB = new TGTextButton(WaveformCreateClose_HF,"Create", WaveformCreateFile_TB_ID),
				   new TGLayoutHints(kLHintsNormal,5,0,8,5));
  WaveformCreateFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformCreateFile_TB->Resize(70,30);
  WaveformCreateFile_TB->ChangeOptions(WaveformCreateFile_TB->GetOptions() | kFixedSize);
  WaveformCreateFile_TB->SetState(kButtonDisabled);
  
  WaveformCreateClose_HF->AddFrame(WaveformCommentFile_TB = new TGTextButton(WaveformCreateClose_HF,"Comment", WaveformCommentFile_TB_ID),
				   new TGLayoutHints(kLHintsNormal,5,0,8,5));
  WaveformCommentFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformCommentFile_TB->Resize(70,30);
  WaveformCommentFile_TB->ChangeOptions(WaveformCommentFile_TB->GetOptions() | kFixedSize);
  WaveformCommentFile_TB->SetState(kButtonDisabled);

  WaveformCreateClose_HF->AddFrame(WaveformCloseFile_TB = new TGTextButton(WaveformCreateClose_HF,"Close", WaveformCloseFile_TB_ID),
				   new TGLayoutHints(kLHintsNormal,5,0,8,5));
  WaveformCloseFile_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  WaveformCloseFile_TB->Resize(70,30);
  WaveformCloseFile_TB->ChangeOptions(WaveformCloseFile_TB->GetOptions() | kFixedSize);
  WaveformCloseFile_TB->SetState(kButtonDisabled);



  WaveformStorage_GF->AddFrame(WaveformStorageEnable_CB = new TGCheckButton(WaveformStorage_GF,"Data stored while checked!", WaveformStorageEnable_CB_ID),
			       new TGLayoutHints(kLHintsNormal,10,5,5,5));
  WaveformStorageEnable_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  WaveformStorageEnable_CB->SetState(kButtonDisabled);

  
  DGDisplayAndControls_VF->AddFrame(Display_VF, new TGLayoutHints(kLHintsCenterX,5,5,5,5));
  DGDisplayAndControls_VF->AddFrame(SubtabFrame, new TGLayoutHints(kLHintsCenterX,5,5,5,5));

  AcquisitionFrame->AddFrame(DGDisplayAndControls_VF, new TGLayoutHints(kLHintsNormal,5,5,0,10));
  DGDisplayAndControls_VF->SetBackgroundColor(ColorManager->Number2Pixel(22));


  // Widgets for saving the spectrum data to file
  
  TGGroupFrame *DGScopeObjectStorage_GF = new TGGroupFrame(DataSubframe, "Data object output", kVerticalFrame);
  DGScopeObjectStorage_GF->SetTitlePos(TGGroupFrame::kCenter);
  DataSubframe->AddFrame(DGScopeObjectStorage_GF, new TGLayoutHints(kLHintsNormal,0,5,5,5));


  TGHorizontalFrame *ObjectType_HF = new TGHorizontalFrame(DGScopeObjectStorage_GF);
  DGScopeObjectStorage_GF->AddFrame(ObjectType_HF, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  TGVButtonGroup *ObjectType_BG = new TGVButtonGroup(ObjectType_HF);
  ObjectType_BG->SetBorderDrawn(false);
  ObjectType_HF->AddFrame(ObjectType_BG, new TGLayoutHints(kLHintsNormal,-7,0,-2,0));
  
  WaveformOutput_RB = new TGRadioButton(ObjectType_BG, "Waveform", WaveformOutput_RB_ID);
  WaveformOutput_RB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  WaveformOutput_RB->SetState(kButtonDown);
  
  SpectrumOutput_RB = new TGRadioButton(ObjectType_BG, "Spectrum", SpectrumOutput_RB_ID);
  SpectrumOutput_RB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleRadioButtons()");

  PSDHistogramOutput_RB = new TGRadioButton(ObjectType_BG, "PSD histo.", PSDHistogramOutput_RB_ID);
  PSDHistogramOutput_RB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  ObjectType_HF->AddFrame(ObjectOutputChannel_CBL = new ADAQComboBoxWithLabel(ObjectType_HF, "", ObjectOutputType_CBL_ID),
			  new TGLayoutHints(kLHintsNormal,0,0,15,0));
  for(uint32_t ch=0; ch<NumDGChannels; ch++)
    ObjectOutputChannel_CBL->GetComboBox()->AddEntry(DGChannelLabels[ch].c_str(),ch);
  ObjectOutputChannel_CBL->GetComboBox()->Select(0);
  ObjectOutputChannel_CBL->GetComboBox()->Resize(80,20);
  ObjectOutputChannel_CBL->GetComboBox()->Connect("Selected(int,int)", "AASubtabSlots", SubtabSlots, "HandleComboBoxes(int,int)");

  
  DGScopeObjectStorage_GF->AddFrame(ObjectOutputFileName_TB = new TGTextButton(DGScopeObjectStorage_GF, "Object file name", ObjectOutputFileName_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,-5,0));
  ObjectOutputFileName_TB->Resize(175, 30);
  ObjectOutputFileName_TB->ChangeOptions(ObjectOutputFileName_TB->GetOptions() | kFixedSize);
  ObjectOutputFileName_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  
  DGScopeObjectStorage_GF->AddFrame(ObjectOutputFileName_TEL = new ADAQTextEntryWithLabel(DGScopeObjectStorage_GF, "", -1),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,0));
  ObjectOutputFileName_TEL->GetEntry()->Resize(175, 25);
  ObjectOutputFileName_TEL->GetEntry()->ChangeOptions(ObjectOutputFileName_TEL->GetOptions() | kFixedSize | kSunkenFrame);
  ObjectOutputFileName_TEL->GetEntry()->SetState(false);
  ObjectOutputFileName_TEL->GetEntry()->SetText("DefaultObject.root");
  
  DGScopeObjectStorage_GF->AddFrame(ObjectSaveWithTimeExtension_CB = new TGCheckButton(DGScopeObjectStorage_GF, "Add time to file name", ObjectSaveWithTimeExtension_CB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeObjectStorage_GF->AddFrame(ObjectSave_TB = new TGTextButton(DGScopeObjectStorage_GF, "Save object data", ObjectSave_TB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,0,5));
  ObjectSave_TB->Resize(175, 30);
  ObjectSave_TB->ChangeOptions(ObjectSave_TB->GetOptions() | kFixedSize);
  ObjectSave_TB->Connect("Clicked()","AASubtabSlots", SubtabSlots, "HandleTextButtons()");
  

  // Widgets for saving the canvas graphics to file
  
  TGGroupFrame *DGScopeCanvasStorage_GF = new TGGroupFrame(DataSubframe, "Canvas outpu", kVerticalFrame);
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
  CanvasFileName_TEL->GetEntry()->SetText("DefaultCanvas.eps");

  DGScopeCanvasStorage_GF->AddFrame(CanvasSaveWithTimeExtension_CB = new TGCheckButton(DGScopeCanvasStorage_GF, "Add time to file name", CanvasSaveWithTimeExtension_CB_ID),
				    new TGLayoutHints(kLHintsNormal, 5,5,5,5));
  
  DGScopeCanvasStorage_GF->AddFrame(CanvasSave_TB = new TGTextButton(DGScopeCanvasStorage_GF, "Save canvas graphics", CanvasSave_TB_ID),
				    new TGLayoutHints(kLHintsNormal,5,5,0,5));
  CanvasSave_TB->Resize(175, 30);
  CanvasSave_TB->ChangeOptions(CanvasSave_TB->GetOptions() | kFixedSize);
  CanvasSave_TB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleTextButtons()");


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
  
  TGGroupFrame *DisplaySettings_GF = new TGGroupFrame(GraphicsSubframe, "Draw Options", kVerticalFrame);
  DisplaySettings_GF->SetTitlePos(TGGroupFrame::kCenter);
  GraphicsSubframe->AddFrame(DisplaySettings_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));


  TGHorizontalFrame *Display_HF0 = new TGHorizontalFrame(DisplaySettings_GF);
  DisplaySettings_GF->AddFrame(Display_HF0, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  Display_HF0->AddFrame(DisplayTrigger_CB = new TGCheckButton(Display_HF0, "Trigger     ", DisplayTrigger_CB_ID),
			new TGLayoutHints(kLHintsNormal, 0,0,2,0));
  DisplayTrigger_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DisplayTrigger_CB->SetState(kButtonDown);
  
  Display_HF0->AddFrame(DisplayBaselineBox_CB = new TGCheckButton(Display_HF0, "Baseline box", DisplayBaselineBox_CB_ID),
		       new TGLayoutHints(kLHintsNormal, 0,0,2,0));
  DisplayBaselineBox_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DisplayBaselineBox_CB->SetState(kButtonDown);


  TGHorizontalFrame *Display_HF1 = new TGHorizontalFrame(DisplaySettings_GF);
  DisplaySettings_GF->AddFrame(Display_HF1, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  Display_HF1->AddFrame(DisplayPSDLimits_CB = new TGCheckButton(Display_HF1, "PSD limits  ", DisplayPSDLimits_CB_ID),
			new TGLayoutHints(kLHintsNormal, 0,0,2,0));
  DisplayPSDLimits_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  Display_HF1->AddFrame(DisplayZLEThreshold_CB = new TGCheckButton(Display_HF1, "ZLE thresh", DisplayZLEThreshold_CB_ID),
			new TGLayoutHints(kLHintsNormal, 0,0,2,0));


  TGHorizontalFrame *Display_HF2 = new TGHorizontalFrame(DisplaySettings_GF);
  DisplaySettings_GF->AddFrame(Display_HF2, new TGLayoutHints(kLHintsNormal, 0,0,0,0));
  
  // ROOT check button to enable/disable plotting of the legend
  Display_HF2->AddFrame(DisplayLegend_CB = new TGCheckButton(Display_HF2, "Legend      ", -1),
			new TGLayoutHints(kLHintsNormal,0,0,2,0));
  DisplayLegend_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  Display_HF2->AddFrame(DisplayGrid_CB = new TGCheckButton(Display_HF2, "Grid", DisplayGrid_CB_ID),
			new TGLayoutHints(kLHintsNormal, 0,0,2,0));
  DisplayGrid_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  DisplayGrid_CB->SetState(kButtonDown);


  TGHorizontalFrame *Display_HF3 = new TGHorizontalFrame(DisplaySettings_GF);
  DisplaySettings_GF->AddFrame(Display_HF3, new TGLayoutHints(kLHintsNormal, 0,0,0,0));

  // ROOT check buttons for specifying if X and Y axes on spectra should be logarithmic
  Display_HF3->AddFrame(DisplayXAxisLog_CB = new TGCheckButton(Display_HF3, "Log X-axis  ", DisplayXAxisLog_CB_ID),
			new TGLayoutHints(kLHintsLeft,0,0,2,0));
  DisplayXAxisLog_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");
  
  Display_HF3->AddFrame(DisplayYAxisLog_CB = new TGCheckButton(Display_HF3, "Log Y-axis", DisplayYAxisLog_CB_ID),
			new TGLayoutHints(kLHintsLeft,0,0,2,0));
  DisplayYAxisLog_CB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleCheckButtons()");

  
  TGGroupFrame *WaveformDrawOptions_GF = new TGGroupFrame(DisplaySettings_GF, "Waveform options", kHorizontalFrame);
  DisplaySettings_GF->AddFrame(WaveformDrawOptions_GF,
			       new TGLayoutHints(kLHintsTop, 0,0,5,0));

  WaveformDrawOptions_GF->AddFrame(DrawWaveformWithLine_RB = new TGRadioButton(WaveformDrawOptions_GF,
									       "Line ",
									       DrawWaveformWithLine_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawWaveformWithLine_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  DrawWaveformWithLine_RB->SetState(kButtonDown);
  
  WaveformDrawOptions_GF->AddFrame(DrawWaveformWithMarkers_RB = new TGRadioButton(WaveformDrawOptions_GF,
										  "Markers ",
										  DrawWaveformWithMarkers_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawWaveformWithMarkers_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  WaveformDrawOptions_GF->AddFrame(DrawWaveformWithBoth_RB = new TGRadioButton(WaveformDrawOptions_GF,
									       "Both",
									       DrawWaveformWithBoth_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawWaveformWithBoth_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  
  TGGroupFrame *SpectrumDrawOptions_GF = new TGGroupFrame(DisplaySettings_GF, "Spectrum options", kHorizontalFrame);
  DisplaySettings_GF->AddFrame(SpectrumDrawOptions_GF,
			       new TGLayoutHints(kLHintsNormal, 0,0,5,0));
  
  SpectrumDrawOptions_GF->AddFrame(DrawSpectrumWithLine_RB = new TGRadioButton(SpectrumDrawOptions_GF,
									       "Line ",
									       DrawSpectrumWithLine_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawSpectrumWithLine_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  DrawSpectrumWithLine_RB->SetState(kButtonDown);
  
  SpectrumDrawOptions_GF->AddFrame(DrawSpectrumWithMarkers_RB = new TGRadioButton(SpectrumDrawOptions_GF,
										  "Markers ",
										  DrawSpectrumWithMarkers_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawSpectrumWithMarkers_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");
  
  SpectrumDrawOptions_GF->AddFrame(DrawSpectrumWithBars_RB = new TGRadioButton(SpectrumDrawOptions_GF,
									       "Bars",
									       DrawSpectrumWithBars_RB_ID),
				   new TGLayoutHints(kLHintsNormal, 0,3,3,-2));
  DrawSpectrumWithBars_RB->Connect("Clicked()", "AASubtabSlots", SubtabSlots, "HandleRadioButtons()");

  
  TGGroupFrame *DisplayControl_GF = new TGGroupFrame(GraphicsSubframe, "Control", kVerticalFrame);
  DisplayControl_GF->SetTitlePos(TGGroupFrame::kCenter);
  GraphicsSubframe->AddFrame(DisplayControl_GF, new TGLayoutHints(kLHintsNormal,5,5,5,5));
  
  DisplayControl_GF->AddFrame(SpectrumRefreshRate_NEL = new ADAQNumberEntryWithLabel(DisplayControl_GF, "Display update freq.", -1),
			      new TGLayoutHints(kLHintsNormal, 0,0,10,0));
  SpectrumRefreshRate_NEL->GetEntry()->SetNumStyle(TGNumberFormat::kNESInteger);
  SpectrumRefreshRate_NEL->GetEntry()->SetNumAttr(TGNumberFormat::kNEAPositive);
  SpectrumRefreshRate_NEL->GetEntry()->Resize(50,20);
  SpectrumRefreshRate_NEL->GetEntry()->SetNumber(100);
  

  TGButtonGroup *DisplayControl_BG = new TGButtonGroup(DisplayControl_GF, "");
  DisplayControl_BG->SetBorderDrawn(false);
  DisplayControl_GF->AddFrame(DisplayControl_BG,
			      new TGLayoutHints(kLHintsNormal, -12,0,-3,0));
  
  DisplayContinuous_RB = new TGRadioButton(DisplayControl_BG, "Continuous (standard rate)", DisplayContinuous_RB_ID);
  DisplayContinuous_RB->SetState(kButtonDown);
  
  DisplayUpdateable_RB = new TGRadioButton(DisplayControl_BG, "Updateable (high rate)", DisplayUpdateable_RB_ID);
  
  DisplayNonUpdateable_RB = new TGRadioButton(DisplayControl_BG, "Waveform storage only!", DisplayNonUpdateable_RB_ID);

  MapSubwindows();
  MapWindow();
}
  

// Perform actions that ensure a safe shutdown and disconnect of the
// AAInterface software from the VME boards
void AAInterface::HandleDisconnectAndTerminate(bool Terminate)
{
  AAVMEManager::GetInstance()->SafelyDisconnectVMEBoards();
  
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
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  const int NumHVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
  
  for(int ch=0; ch<NumHVChannels; ch++){
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
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  const int NumDGChannels = TheVMEManager->GetDGManager()->GetNumChannels();

  for(Int_t ch=0; ch<NumDGChannels; ch++){
    DGChEnable_CB[ch]->SetState(ButtonState);
    DGChPosPolarity_RB[ch]->SetState(ButtonState);
    DGChNegPolarity_RB[ch]->SetState(ButtonState);
    DGChDCOffset_NEL[ch]->GetEntry()->SetState(WidgetState);
    DGChTriggerThreshold_NEL[ch]->GetEntry()->SetState(WidgetState);
    if(DGStandardFW_RB->IsDown()){
      DGChZLEThreshold_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChZLEBackward_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChZLEForward_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChZLEPosLogic_RB[ch]->SetState(ButtonState);
      DGChZLENegLogic_RB[ch]->SetState(ButtonState);
      DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChPSDTotalStart_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChPSDTotalStop_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChPSDTailStart_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChPSDTailStop_NEL[ch]->GetEntry()->SetState(WidgetState);
    }
    else if(DGPSDFW_RB->IsDown()){
      DGChRecordLength_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChBaselineSamples_CBL[ch]->GetComboBox()->SetEnabled(WidgetState);
      DGChChargeSensitivity_CBL[ch]->GetComboBox()->SetEnabled(WidgetState);
      //DGChPSDCut_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChTriggerConfig_CBL[ch]->GetComboBox()->SetEnabled(WidgetState);
      DGChTriggerValidation_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChShortGate_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChLongGate_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChPreTrigger_NEL[ch]->GetEntry()->SetState(WidgetState);
      DGChGateOffset_NEL[ch]->GetEntry()->SetState(WidgetState);
    }
  }

  AQWaveform_RB->SetEnabled(WidgetState);
  AQSpectrum_RB->SetEnabled(WidgetState);
  AQPSDHistogram_RB->SetEnabled(WidgetState);

  DGTriggerType_CBL->GetComboBox()->SetEnabled(WidgetState);
  if(DGStandardFW_RB->IsDown()){
     DGTriggerEdge_CBL->GetComboBox()->SetEnabled(WidgetState);
  }
  else if(DGPSDFW_RB->IsDown()){
    DGPSDTriggerHoldoff_NEL->GetEntry()->SetState(WidgetState);
  }
  DGTriggerCoincidenceEnable_CB->SetState(ButtonState);

  DGAcquisitionControl_CBL->GetComboBox()->SetEnabled(WidgetState);
  if(DGStandardFW_RB->IsDown()){
    DGRecordLength_NEL->GetEntry()->SetState(WidgetState);
    DGPostTrigger_NEL->GetEntry()->SetState(WidgetState);
  }
  else if(DGPSDFW_RB->IsDown()){
    DGPSDMode_CBL->GetComboBox()->SetEnabled(WidgetState);
    DGPSDListAnalysis_RB->SetState(ButtonState);
    DGPSDWaveformAnalysis_RB->SetState(ButtonState);
  }

  DGEventsBeforeReadout_NEL->GetEntry()->SetState(WidgetState);
  AQDataReductionEnable_CB->SetState(ButtonState);
  AQDataReductionFactor_NEL->GetEntry()->SetState(WidgetState);
  DGZLEEnable_CB->SetState(ButtonState);

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
  DisplayContinuous_RB->SetEnabled(WidgetState);
  DisplayUpdateable_RB->SetEnabled(WidgetState);
  DisplayNonUpdateable_RB->SetEnabled(WidgetState);


  // The following widgets have special settings depending on
  // the acquisition state
  
  // Acquisition is turning ON
  if(WidgetState == false){
    AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOn));
    AQStartStop_TB->SetText("Acquiring");
    
    WaveformCreateFile_TB->SetState(kButtonUp);
  }

  // Acquisition is turning OFF
  else{
    AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
    AQStartStop_TB->SetText("Stopped");
    
    if(WaveformCreateFile_TB->GetString() == "File open!"){
      WaveformCreateFile_TB->SetText("Create");
      WaveformCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
      WaveformCreateFile_TB->SetForegroundColor(ColorManager->Number2Pixel(kBlack));
    }
    WaveformCreateFile_TB->SetState(kButtonDisabled);
    WaveformCommentFile_TB->SetState(kButtonDisabled);
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
  SpectrumCalibrationUnit_CBL->GetComboBox()->SetEnabled(WidgetState);
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
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  // If the "Save" settings text button is highlighted (indicating a
  // settings save has occured at some point in the past) and the
  // "auto save" check box is not checked then reset it since setting
  // may have changed
  if(SaveSettingsToFile_TB->GetString() == "Saved!" and
     !AutoSaveSettings_CB->IsDown()){
    SaveSettingsToFile_TB->SetText("Save");
    SaveSettingsToFile_TB->SetForegroundColor(ColorManager->Number2Pixel(kBlack));
    SaveSettingsToFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
  }
  
  // The "Settings" and "VMEConnection" widgets are built regardless
  // of program state so we can always save their state; however, the
  // AASettings object is DG and HV channel dependent, which is not
  // known until _after_ the VME connection has been made. Thus, if
  // the connection has not occured we will create a temporary
  // AASettings object to hold the settings for accessible widgets.
  
  if(!InterfaceBuildComplete)
    TheSettings = new AASettings(0, 0);
  
  //////////////////
  // Settings tab //
  //////////////////
  
  TheSettings->SettingsFileName = SettingsFileName_TEL->GetEntry()->GetText();
  TheSettings->AutoSaveSettings = AutoSaveSettings_CB->IsDown();
  
  ////////////////////////
  // VME connection tab //
  ////////////////////////
  
  for(Int_t board=0; board<NumBoards; board++){
    TheSettings->BoardType[board] = BoardType_CBL[board]->GetComboBox()->GetSelected();
    if(board != zBR)
      TheSettings->BoardAddress[board] = BoardAddress_NEF[board]->GetEntry()->GetIntNumber();
    TheSettings->BoardLinkNumber[board] = BoardLinkNumber_NEL[board]->GetEntry()->GetIntNumber();
    
    if(BoardEnable_TB[board]->GetString() == "Board enabled")
      TheSettings->BoardEnable[board] = true;
    else if(BoardEnable_TB[board]->GetString() == "Board disabled")
      TheSettings->BoardEnable[board] = false;
  }
  
  TheSettings->STDFirmware = DGStandardFW_RB->IsDown();
  TheSettings->PSDFirmware = DGPSDFW_RB->IsDown();


  // The following widgets are built after a connection has been
  // established; test to ensure this has occured before attempting to
  // access the objects
  
  if(!InterfaceBuildComplete)
    return;
  
  //////////////////////
  // High voltage tab //
  //////////////////////
  
  if(TheSettings->BoardEnable[zHV]){
    
    const Int_t NumHVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    
    for(Int_t ch=0; ch<NumHVChannels; ch++){
      TheSettings->HVChVoltage[ch] = HVChVoltage_NEL[ch]->GetEntry()->GetIntNumber();
      TheSettings->HVChCurrent[ch] = HVChCurrent_NEL[ch]->GetEntry()->GetIntNumber();
    }
  }

  
  /////////////////////
  // Acquisition tab //
  /////////////////////

  if(TheSettings->BoardEnable[zDG]){
    
    const Int_t NumDGChannels = TheVMEManager->GetDGManager()->GetNumChannels();
    
    // Acquisition channel 
    for(Int_t ch=0; ch<NumDGChannels; ch++){
      TheSettings->ChEnable[ch] = DGChEnable_CB[ch]->IsDown();
      TheSettings->ChPosPolarity[ch] = DGChPosPolarity_RB[ch]->IsDown();
      TheSettings->ChNegPolarity[ch] = DGChNegPolarity_RB[ch]->IsDown();
      TheSettings->ChDCOffset[ch] = DGChDCOffset_NEL[ch]->GetEntry()->GetHexNumber();
      TheSettings->ChTriggerThreshold[ch] = DGChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
      if(DGStandardFW_RB->IsDown()){
	TheSettings->ChZLEThreshold[ch] = DGChZLEThreshold_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChZLEForward[ch] = DGChZLEForward_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChZLEBackward[ch] = DGChZLEBackward_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChZLEPosLogic[ch] = DGChZLEPosLogic_RB[ch]->IsDown();
	TheSettings->ChZLENegLogic[ch] = DGChZLENegLogic_RB[ch]->IsDown();
	TheSettings->ChBaselineCalcMin[ch] = DGChBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChBaselineCalcMax[ch] = DGChBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChPSDTotalStart[ch] = DGChPSDTotalStart_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChPSDTotalStop[ch] = DGChPSDTotalStop_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChPSDTailStart[ch] = DGChPSDTailStart_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChPSDTailStop[ch] = DGChPSDTailStop_NEL[ch]->GetEntry()->GetIntNumber();
      }
      else if(DGPSDFW_RB->IsDown()){
	TheSettings->ChRecordLength[ch] = DGChRecordLength_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChBaselineSamples[ch] = DGChBaselineSamples_CBL[ch]->GetComboBox()->GetSelected();
	TheSettings->ChChargeSensitivity[ch] = DGChChargeSensitivity_CBL[ch]->GetComboBox()->GetSelected();
	TheSettings->ChPSDCut[ch] = DGChPSDCut_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChTriggerConfig[ch] = DGChTriggerConfig_CBL[ch]->GetComboBox()->GetSelected();
	TheSettings->ChTriggerValidation[ch] = DGChTriggerValidation_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChShortGate[ch] = DGChShortGate_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChLongGate[ch] = DGChLongGate_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChPreTrigger[ch] = DGChPreTrigger_NEL[ch]->GetEntry()->GetIntNumber();
	TheSettings->ChGateOffset[ch] = DGChGateOffset_NEL[ch]->GetEntry()->GetIntNumber();
      }
    }
  
    TheSettings->HorizontalSliderPtr = DisplayHorizontalScale_THS->GetPointerPosition();
  
    Float_t Min = 0., Max = 0.;
  
    DisplayHorizontalScale_THS->GetPosition(Min, Max);
    TheSettings->HorizontalSliderMin = Min;
    TheSettings->HorizontalSliderMax = Max;
  
    DisplayVerticalScale_DVS->GetPosition(Min, Max);
    TheSettings->VerticalSliderMin = Min;
    TheSettings->VerticalSliderMax = Max;

    //////////////////////////
    // Data acquisition subtab

    // Scope display
    TheSettings->WaveformMode = AQWaveform_RB->IsDown();
    TheSettings->SpectrumMode = AQSpectrum_RB->IsDown();
    TheSettings->PSDMode = AQPSDHistogram_RB->IsDown();
    
    // Trigger control settings
    TheSettings->TriggerType = DGTriggerType_CBL->GetComboBox()->GetSelected();
    TheSettings->TriggerTypeName = DGTriggerType_CBL->GetComboBox()->GetSelectedEntry()->GetTitle();

    if(DGStandardFW_RB->IsDown()){
      TheSettings->TriggerEdge = DGTriggerEdge_CBL->GetComboBox()->GetSelected();
      TheSettings->TriggerEdgeName = DGTriggerEdge_CBL->GetComboBox()->GetSelectedEntry()->GetTitle();
    }
    else if(DGPSDFW_RB->IsDown()){
      TheSettings->PSDTriggerHoldoff = DGPSDTriggerHoldoff_NEL->GetEntry()->GetIntNumber();
    }
  
    TheSettings->TriggerCoincidenceEnable = DGTriggerCoincidenceEnable_CB->IsDown();
    TheSettings->TriggerCoincidenceLevel = DGTriggerCoincidenceLevel_CBL->GetComboBox()->GetSelected();

    // Acquisition
    TheSettings->AcquisitionControl = DGAcquisitionControl_CBL->GetComboBox()->GetSelected();
    TheSettings->AcquisitionControlName = DGAcquisitionControl_CBL->GetComboBox()->GetSelectedEntry()->GetTitle();

    if(DGStandardFW_RB->IsDown()){
      TheSettings->RecordLength = DGRecordLength_NEL->GetEntry()->GetIntNumber();
      TheSettings->PostTrigger = DGPostTrigger_NEL->GetEntry()->GetIntNumber();
    }
    else{
      TheSettings->PSDOperationMode = DGPSDMode_CBL->GetComboBox()->GetSelected();
      TheSettings->PSDListAnalysis = DGPSDListAnalysis_RB->IsDown();
      TheSettings->PSDWaveformAnalysis = DGPSDWaveformAnalysis_RB->IsDown();
    }
  
    TheSettings->AcquisitionTime = AQTime_NEL->GetEntry()->GetIntNumber();

    // Readout
    TheSettings->EventsBeforeReadout = DGEventsBeforeReadout_NEL->GetEntry()->GetIntNumber();
    TheSettings->DataReductionEnable = AQDataReductionEnable_CB->IsDown();
    TheSettings->DataReductionFactor = AQDataReductionFactor_NEL->GetEntry()->GetIntNumber();
    TheSettings->ZeroSuppressionEnable = DGZLEEnable_CB->IsDown();


    ///////////////////////
    // Pulse spectra subtab

    TheSettings->SpectrumChannel = SpectrumChannel_CBL->GetComboBox()->GetSelected();
    TheSettings->SpectrumNumBins = SpectrumNumBins_NEL->GetEntry()->GetIntNumber();
    TheSettings->SpectrumMinBin = SpectrumMinBin_NEL->GetEntry()->GetNumber();
    TheSettings->SpectrumMaxBin = SpectrumMaxBin_NEL->GetEntry()->GetNumber();

    TheSettings->SpectrumPulseHeight = SpectrumPulseHeight_RB->IsDown();
    TheSettings->SpectrumPulseArea = SpectrumPulseArea_RB->IsDown();

    TheSettings->LDEnable = SpectrumLDEnable_CB->IsDown();
    TheSettings->SpectrumLLD = SpectrumLLD_NEL->GetEntry()->GetIntNumber();
    TheSettings->SpectrumULD = SpectrumULD_NEL->GetEntry()->GetIntNumber();
    TheSettings->LDTrigger = SpectrumLDTrigger_CB->IsDown();

    TheSettings->LDChannel = SpectrumLDTriggerChannel_CBL->GetComboBox()->GetSelected();
  
    TheSettings->SpectrumCalibrationEnable = SpectrumCalibration_CB->IsDown();
    TheSettings->SpectrumCalibrationUseSlider = SpectrumUseCalibrationSlider_CB->IsDown();
    TheSettings->SpectrumCalibrationUnit = SpectrumCalibrationUnit_CBL->GetComboBox()->GetSelectedEntry()->GetTitle();


    //////////////////////////////
    // Pulse discrimination subtab 

    TheSettings->PSDChannel = PSDChannel_CBL->GetComboBox()->GetSelected();
    TheSettings->PSDYAxisTail = PSDYAxisTail_RB->IsDown();
    TheSettings->PSDYAxisTailTotal = PSDYAxisTailTotal_RB->IsDown();
    TheSettings->PSDThreshold = PSDThreshold_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTotalBins = PSDTotalBins_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTotalMinBin = PSDTotalMinBin_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTotalMaxBin = PSDTotalMaxBin_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTailBins = PSDTailBins_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTailMinBin = PSDTailMinBin_NEL->GetEntry()->GetNumber();
    TheSettings->PSDTailMaxBin = PSDTailMaxBin_NEL->GetEntry()->GetNumber();


    //////////////////////////
    // Display graphics subtab

    TheSettings->DisplayTitlesEnable = DisplayTitlesEnable_CB->IsDown();

    TheSettings->DisplayTitle = DisplayTitle_TEL->GetEntry()->GetText();

    TheSettings->DisplayXTitle = DisplayXTitle_TEL->GetEntry()->GetText();
    TheSettings->DisplayXTitleSize = DisplayXTitleSize_NEL->GetEntry()->GetNumber();
    TheSettings->DisplayXTitleOffset = DisplayXTitleOffset_NEL->GetEntry()->GetNumber();

    TheSettings->DisplayYTitle = DisplayYTitle_TEL->GetEntry()->GetText();
    TheSettings->DisplayYTitleSize = DisplayYTitleSize_NEL->GetEntry()->GetNumber();
    TheSettings->DisplayYTitleOffset = DisplayYTitleOffset_NEL->GetEntry()->GetNumber();
	
    TheSettings->DisplayTrigger = DisplayTrigger_CB->IsDown();
    TheSettings->DisplayBaselineBox = DisplayBaselineBox_CB->IsDown();
    TheSettings->DisplayPSDLimits = DisplayPSDLimits_CB->IsDown();
    TheSettings->DisplayZLEThreshold = DisplayZLEThreshold_CB->IsDown();
    TheSettings->DisplayLegend = DisplayLegend_CB->IsDown();
    TheSettings->DisplayGrid = DisplayGrid_CB->IsDown();
    TheSettings->DisplayXAxisInLog = DisplayXAxisLog_CB->IsDown();
    TheSettings->DisplayYAxisInLog = DisplayYAxisLog_CB->IsDown();
  
    TheSettings->WaveformWithLine = DrawWaveformWithLine_RB->IsDown();
    TheSettings->WaveformWithMarkers = DrawWaveformWithMarkers_RB->IsDown();
    TheSettings->WaveformWithBoth = DrawWaveformWithBoth_RB->IsDown();
  
    TheSettings->SpectrumWithLine = DrawSpectrumWithLine_RB->IsDown();
    TheSettings->SpectrumWithMarkers = DrawSpectrumWithMarkers_RB->IsDown();
    TheSettings->SpectrumWithBars = DrawSpectrumWithBars_RB->IsDown();
  
    TheSettings->SpectrumRefreshRate = SpectrumRefreshRate_NEL->GetEntry()->GetIntNumber();

    TheSettings->DisplayContinuous = DisplayContinuous_RB->IsDown();
    TheSettings->DisplayUpdateable = DisplayUpdateable_RB->IsDown();
    TheSettings->DisplayNonUpdateable = DisplayNonUpdateable_RB->IsDown();
  

    ////////////////////////////
    // Persistent storage subtab

    TheSettings->WaveformStorageEnable = WaveformStorageEnable_CB->IsDown();
    TheSettings->WaveformStoreRaw = WaveformStoreRaw_CB->IsDown();
    TheSettings->WaveformStoreEnergyData = WaveformStoreEnergyData_CB->IsDown();
    TheSettings->WaveformStorePSDData= WaveformStorePSDData_CB->IsDown();

    TheSettings->ObjectSaveWithTimeExtension = ObjectSaveWithTimeExtension_CB->IsDown();
    TheSettings->CanvasSaveWithTimeExtension = CanvasSaveWithTimeExtension_CB->IsDown();

    ///////////////////////////////
    // Acquisition disabled widgets

    // Because of the frustrating way ROOT implements button behavior,
    // buttons that are intentionally disabled while acquisition is on
    // (to prevent the user from changing settings during acquisition)
    // must be tested for the special disabled-and-selected
    // enumerator. This is necessary since widgets that are activated
    // during acquisition will call this method and, unless the
    // following value readouts are performed, the settings for the
    // disabled will be wrong. This results in bad and unexpected
    // behavior. Thus, we must ensure that all of the following buttons
    // are disabled during acquisition!

    Bool_t AcquisitionOn = AAAcquisitionManager::GetInstance()->GetAcquisitionEnable();

    if(AcquisitionOn){
      for(Int_t ch=0; ch<NumDGChannels; ch++){
	TheSettings->ChEnable[ch] = DGChEnable_CB[ch]->IsDisabledAndSelected();
	TheSettings->ChPosPolarity[ch] = DGChPosPolarity_RB[ch]->IsDisabledAndSelected();
	TheSettings->ChNegPolarity[ch] = DGChNegPolarity_RB[ch]->IsDisabledAndSelected();
	if(DGStandardFW_RB->IsDown()){
	  TheSettings->ChZLEPosLogic[ch] = DGChZLEPosLogic_RB[ch]->IsDisabledAndSelected();
	  TheSettings->ChZLENegLogic[ch] = DGChZLENegLogic_RB[ch]->IsDisabledAndSelected();
	}
	else if(DGPSDFW_RB->IsDown()){
	}
      }

      TheSettings->WaveformMode = AQWaveform_RB->IsDisabledAndSelected();
      TheSettings->SpectrumMode = AQSpectrum_RB->IsDisabledAndSelected();
      TheSettings->PSDMode = AQPSDHistogram_RB->IsDisabledAndSelected();

      TheSettings->TriggerCoincidenceEnable = DGTriggerCoincidenceEnable_CB->IsDisabledAndSelected();

      if(DGPSDFW_RB->IsDown()){
	TheSettings->PSDMode = AQPSDHistogram_RB->IsDisabledAndSelected();
	TheSettings->PSDListAnalysis = DGPSDListAnalysis_RB->IsDisabledAndSelected();
	TheSettings->PSDWaveformAnalysis = DGPSDWaveformAnalysis_RB->IsDisabledAndSelected();
      }

      TheSettings->DataReductionEnable = AQDataReductionEnable_CB->IsDisabledAndSelected();
      TheSettings->ZeroSuppressionEnable = DGZLEEnable_CB->IsDisabledAndSelected();

      TheSettings->SpectrumPulseHeight = SpectrumPulseHeight_RB->IsDisabledAndSelected();
      TheSettings->SpectrumPulseArea = SpectrumPulseArea_RB->IsDisabledAndSelected();
      TheSettings->LDEnable = SpectrumLDEnable_CB->IsDown();
      TheSettings->LDTrigger = SpectrumLDTrigger_CB->IsDisabledAndSelected();

      TheSettings->PSDYAxisTail = PSDYAxisTail_RB->IsDisabledAndSelected();
      TheSettings->PSDYAxisTailTotal = PSDYAxisTailTotal_RB->IsDisabledAndSelected();

      TheSettings->WaveformStoreRaw = WaveformStoreRaw_CB->IsDisabledAndSelected();
      TheSettings->WaveformStoreEnergyData = WaveformStoreEnergyData_CB->IsDisabledAndSelected();
      TheSettings->WaveformStorePSDData = WaveformStorePSDData_CB->IsDisabledAndSelected();

      TheSettings->DisplayContinuous = DisplayContinuous_RB->IsDisabledAndSelected();
      TheSettings->DisplayUpdateable = DisplayUpdateable_RB->IsDisabledAndSelected();
      TheSettings->DisplayNonUpdateable = DisplayNonUpdateable_RB->IsDisabledAndSelected();
    }
  }
  
  if(AutoSaveSettings)
    SaveSettingsToFile();
}


void AAInterface::SaveActiveSettings()
{
  TheSettings->AcquisitionTime = AQTime_NEL->GetEntry()->GetIntNumber();
  
  TheSettings->WaveformStoreRaw = WaveformStoreRaw_CB->IsDown();
  TheSettings->WaveformStoreEnergyData = WaveformStoreEnergyData_CB->IsDown();
  TheSettings->WaveformStorePSDData= WaveformStorePSDData_CB->IsDown();
}


void AAInterface::SaveSettingsToFile()
{
  // Save all interface settings to the AASettings object
  SaveSettings();
  
  // Write the AASettings object to the settings ROOT file
  TFile *SettingsFile = new TFile(SettingsFileName.c_str(), "recreate");
  
  TheSettings->Write("TheSettings");
  SettingsFile->Close();
}


void AAInterface::LoadSettingsFromFile()
{
  // Open the ROOT file containing an ADAQAcquisition AASettings object
  TFile *SettingsFile = new TFile(SettingsFileName.c_str(), "read");
  
  // Load the settings object that will be used for readout into the
  // widgets; note that a new AASettings object named 'TheSettings' is
  // used here to prevent overwriting the class data member
  AASettings *TheSettings = (AASettings *)SettingsFile->Get("TheSettings");

  //////////////////
  // Settings tab //
  //////////////////

  if(TheSettings->AutoSaveSettings)
    AutoSaveSettings_CB->SetState(kButtonDown);
  else
    AutoSaveSettings_CB->SetState(kButtonUp);
  
  
  ////////////////////
  // Connection tab //
  ////////////////////

  for(Int_t board=0; board<3; board++){
    BoardType_CBL[board]->GetComboBox()->Select(TheSettings->BoardType[board]);
    
    if(board != 0)
      BoardAddress_NEF[board]->GetEntry()->SetHexNumber(TheSettings->BoardAddress[board]);
    
    BoardLinkNumber_NEL[board]->GetEntry()->SetIntNumber(TheSettings->BoardLinkNumber[board]);
    
    if(TheSettings->BoardEnable[board]){
      BoardEnable_TB[board]->SetText("Board enabled");
      BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOn));
      BoardEnable_TB[board]->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
      BoardEnable_TB[board]->ChangeOptions(BoardEnable_TB[board]->GetOptions() | kFixedSize);
    }
    else{
      BoardEnable_TB[board]->SetText("Board disabled");
      BoardEnable_TB[board]->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
      BoardEnable_TB[board]->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
      BoardEnable_TB[board]->ChangeOptions(BoardEnable_TB[board]->GetOptions() | kFixedSize);
    }
  }
  
  if(TheSettings->STDFirmware){
    DGStandardFW_RB->SetState(kButtonDown);
    DGPSDFW_RB->SetState(kButtonUp);
  }
  else if(TheSettings->PSDFirmware){
    DGStandardFW_RB->SetState(kButtonUp);
    DGPSDFW_RB->SetState(kButtonDown);
  }
  
  // If the full interface (e.g. the secondary frames that are device
  // specific) has not been built then return to prevent seg faults

  if(!InterfaceBuildComplete)
    return;

  // Get the VME manager
  AAVMEManager *VMEManager = AAVMEManager::GetInstance();


  //////////////////////
  // High voltage tab //
  //////////////////////

  size_t HVChSize = TheSettings->HVChVoltage.size();
  
  if(VMEManager->GetHVEnable() and HVChSize != 0){
    const Int_t NumHVChannels = VMEManager->GetHVManager()->GetNumChannels();
    
    for(Int_t ch=0; ch<NumHVChannels; ch++){
      // To be implemented
    }
  }

  
  /////////////////////
  // Acquisition tab //
  /////////////////////
  
  size_t DGChSize = TheSettings->ChEnable.size();

  if(VMEManager->GetDGEnable() and DGChSize != 0){
    
    // Channel-specific settings
    
    const Int_t NumDGChannels = AAVMEManager::GetInstance()->GetDGManager()->GetNumChannels();
    
    for(Int_t ch=0; ch<NumDGChannels; ch++){
      
      if(TheSettings->ChEnable[ch])
	DGChEnable_CB[ch]->SetState(kButtonDown);
      else
	DGChEnable_CB[ch]->SetState(kButtonUp);
    
      if(TheSettings->ChPosPolarity[ch])
	DGChPosPolarity_RB[ch]->SetState(kButtonDown);
      else
	DGChPosPolarity_RB[ch]->SetState(kButtonUp);
    
      if(TheSettings->ChNegPolarity[ch])
	DGChNegPolarity_RB[ch]->SetState(kButtonDown);
      else
	DGChNegPolarity_RB[ch]->SetState(kButtonUp);

      DGChDCOffset_NEL[ch]->GetEntry()->SetHexNumber(TheSettings->ChDCOffset[ch]);
      DGChTriggerThreshold_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChTriggerThreshold[ch]);

      if(TheSettings->STDFirmware){
	DGChZLEThreshold_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChZLEThreshold[ch]);
	DGChZLEForward_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChZLEForward[ch]);
	DGChZLEBackward_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChZLEBackward[ch]);

	if(TheSettings->ChZLEPosLogic[ch])
	  DGChZLEPosLogic_RB[ch]->SetState(kButtonDown);
	else
	  DGChZLEPosLogic_RB[ch]->SetState(kButtonUp);
      
	if(TheSettings->ChZLENegLogic[ch])
	  DGChZLENegLogic_RB[ch]->SetState(kButtonDown);
	else
	  DGChZLENegLogic_RB[ch]->SetState(kButtonUp);

	DGChBaselineCalcMin_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChBaselineCalcMin[ch]);
	DGChBaselineCalcMax_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChBaselineCalcMax[ch]);
	DGChPSDTotalStart_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPSDTotalStart[ch]);
	DGChPSDTotalStop_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPSDTotalStop[ch]);
	DGChPSDTailStart_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPSDTailStart[ch]);
	DGChPSDTailStop_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPSDTotalStop[ch]);
      }
      else if(TheSettings->PSDFirmware){
	DGChRecordLength_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChRecordLength[ch]);
	DGChBaselineSamples_CBL[ch]->GetComboBox()->Select(TheSettings->ChBaselineSamples[ch]);
	DGChChargeSensitivity_CBL[ch]->GetComboBox()->Select(TheSettings->ChChargeSensitivity[ch]);
	DGChPSDCut_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPSDCut[ch]);
	DGChTriggerConfig_CBL[ch]->GetComboBox()->Select(TheSettings->ChTriggerConfig[ch]);
	DGChTriggerValidation_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChTriggerValidation[ch]);
	DGChShortGate_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChShortGate[ch]);
	DGChLongGate_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChLongGate[ch]);
	DGChPreTrigger_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChPreTrigger[ch]);
	DGChGateOffset_NEL[ch]->GetEntry()->SetIntNumber(TheSettings->ChGateOffset[ch]);
      }
    }
  
    // Acquisition display type

    if(TheSettings->WaveformMode){
      AQWaveform_RB->SetState(kButtonDown);
      AQSpectrum_RB->SetState(kButtonUp);
      AQPSDHistogram_RB->SetState(kButtonUp);
    }
    else if(TheSettings->SpectrumMode){
      AQWaveform_RB->SetState(kButtonUp);
      AQSpectrum_RB->SetState(kButtonDown);
      AQPSDHistogram_RB->SetState(kButtonUp);
    }
    else if(TheSettings->PSDMode){
      AQWaveform_RB->SetState(kButtonUp);
      AQSpectrum_RB->SetState(kButtonUp);
      AQPSDHistogram_RB->SetState(kButtonDown);
    }

    // Trigger control

    DGTriggerType_CBL->GetComboBox()->Select(TheSettings->TriggerType);

    if(TheSettings->STDFirmware)
      DGTriggerEdge_CBL->GetComboBox()->Select(TheSettings->TriggerEdge);
    else if(TheSettings->PSDFirmware)
      DGPSDTriggerHoldoff_NEL->GetEntry()->SetIntNumber(TheSettings->PSDTriggerHoldoff);

    if(TheSettings->TriggerCoincidenceEnable)
      DGTriggerCoincidenceEnable_CB->SetState(kButtonDown);
    else
      DGTriggerCoincidenceEnable_CB->SetState(kButtonUp);

    DGTriggerCoincidenceLevel_CBL->GetComboBox()->Select(TheSettings->TriggerCoincidenceLevel);

    // Acquisition

    DGAcquisitionControl_CBL->GetComboBox()->Select(TheSettings->AcquisitionControl);
  
    if(TheSettings->STDFirmware){
      DGRecordLength_NEL->GetEntry()->SetIntNumber(TheSettings->RecordLength);
      DGPostTrigger_NEL->GetEntry()->SetIntNumber(TheSettings->PostTrigger);
    }
    else if(TheSettings->PSDFirmware){
      DGPSDMode_CBL->GetComboBox()->Select(TheSettings->PSDOperationMode);
    
      if(TheSettings->PSDListAnalysis){
	DGPSDListAnalysis_RB->SetState(kButtonDown);
	DGPSDWaveformAnalysis_RB->SetState(kButtonUp);
      }
      else{
	DGPSDWaveformAnalysis_RB->SetState(kButtonUp);
	DGPSDWaveformAnalysis_RB->SetState(kButtonDown);
      }
    }
  
    AQTime_NEL->GetEntry()->SetIntNumber(TheSettings->AcquisitionTime);

    // Readout

    DGEventsBeforeReadout_NEL->GetEntry()->SetIntNumber(TheSettings->EventsBeforeReadout);

    if(TheSettings->DataReductionEnable)
      AQDataReductionEnable_CB->SetState(kButtonDown);
    else
      AQDataReductionEnable_CB->SetState(kButtonUp);
    
    AQDataReductionFactor_NEL->GetEntry()->SetIntNumber(TheSettings->DataReductionFactor);

    if(TheSettings->ZeroSuppressionEnable)
      DGZLEEnable_CB->SetState(kButtonDown);
    else
      DGZLEEnable_CB->SetState(kButtonUp);

    ////////////////
    // Pulse spectra

    SpectrumChannel_CBL->GetComboBox()->Select(TheSettings->SpectrumChannel,false);

    SpectrumNumBins_NEL->GetEntry()->SetIntNumber(TheSettings->SpectrumNumBins);
    SpectrumMinBin_NEL->GetEntry()->SetNumber(TheSettings->SpectrumMinBin);
    SpectrumMaxBin_NEL->GetEntry()->SetNumber(TheSettings->SpectrumMaxBin);

    if(TheSettings->SpectrumPulseHeight){
      SpectrumPulseHeight_RB->SetState(kButtonDown);
      SpectrumPulseArea_RB->SetState(kButtonUp);
    }
    else{
      SpectrumPulseHeight_RB->SetState(kButtonUp);
      SpectrumPulseArea_RB->SetState(kButtonDown);
    }
  
    if(TheSettings->LDEnable)
      SpectrumLDEnable_CB->SetState(kButtonDown);
    else
      SpectrumLDEnable_CB->SetState(kButtonUp);
  
    SpectrumLLD_NEL->GetEntry()->SetIntNumber(TheSettings->SpectrumLLD);
    SpectrumULD_NEL->GetEntry()->SetIntNumber(TheSettings->SpectrumULD);

    if(TheSettings->LDTrigger)
      SpectrumLDTrigger_CB->SetState(kButtonDown);
    else
      SpectrumLDTrigger_CB->SetState(kButtonUp);

    SpectrumLDTriggerChannel_CBL->GetComboBox()->Select(TheSettings->LDChannel, false);

    if(TheSettings->SpectrumCalibrationEnable)
      SpectrumCalibration_CB->SetState(kButtonDown);
    else
      SpectrumCalibration_CB->SetState(kButtonUp);

    if(TheSettings->SpectrumCalibrationUseSlider)
      SpectrumUseCalibrationSlider_CB->SetState(kButtonDown);
    else
      SpectrumUseCalibrationSlider_CB->SetState(kButtonUp);

    // TheSettings->SpectrumCalibrationUnit = SpectrumCalibrationUnit_CBL->GetComboBox()->GetSelectedEntry()->GetTitle();

    ///////////////////////
    // Pulse discrimination

    PSDChannel_CBL->GetComboBox()->Select(TheSettings->PSDChannel, false);

    if(TheSettings->PSDYAxisTail){
      PSDYAxisTail_RB->SetState(kButtonDown);
      PSDYAxisTailTotal_RB->SetState(kButtonUp);
    }
    else{
      PSDYAxisTail_RB->SetState(kButtonUp);
      PSDYAxisTailTotal_RB->SetState(kButtonDown);
    }

    PSDThreshold_NEL->GetEntry()->SetNumber(TheSettings->PSDThreshold);
    PSDTotalBins_NEL->GetEntry()->SetNumber(TheSettings->PSDTotalBins);
    PSDTotalMinBin_NEL->GetEntry()->SetNumber(TheSettings->PSDTotalMinBin);
    PSDTotalMaxBin_NEL->GetEntry()->SetNumber(TheSettings->PSDTotalMaxBin);
    PSDTailBins_NEL->GetEntry()->SetNumber(TheSettings->PSDTailBins);
    PSDTailMinBin_NEL->GetEntry()->SetNumber(TheSettings->PSDTailMinBin);
    PSDTailMaxBin_NEL->GetEntry()->SetNumber(TheSettings->PSDTailMaxBin);

    /////////////////////
    // Persistent storage
  
    if(TheSettings->WaveformStorageEnable)
      WaveformStorageEnable_CB->SetState(kButtonDown);
    else
      WaveformStorageEnable_CB->SetState(kButtonUp);
  
    if(TheSettings->WaveformStoreRaw)
      WaveformStoreRaw_CB->SetState(kButtonDown);
    else
      WaveformStoreRaw_CB->SetState(kButtonUp);

    if(TheSettings->WaveformStoreEnergyData)
      WaveformStoreEnergyData_CB->SetState(kButtonDown);
    else
      WaveformStoreEnergyData_CB->SetState(kButtonUp);

    if(TheSettings->WaveformStorePSDData)
      WaveformStorePSDData_CB->SetState(kButtonDown);
    else
      WaveformStorePSDData_CB->SetState(kButtonUp);

    if(TheSettings->ObjectSaveWithTimeExtension)
      ObjectSaveWithTimeExtension_CB->SetState(kButtonDown);
    else
      ObjectSaveWithTimeExtension_CB->SetState(kButtonUp);

    if(TheSettings->CanvasSaveWithTimeExtension)
      CanvasSaveWithTimeExtension_CB->SetState(kButtonDown);
    else
      CanvasSaveWithTimeExtension_CB->SetState(kButtonUp);
  }

  // Close the transient settings ROOT file
  SettingsFile->Close();
}


void AAInterface::UpdateAQTimer(int TimeRemaining)
{ AQTimer_NEFL->GetEntry()->SetNumber(TimeRemaining); }


void AAInterface::UpdateAfterAQTimerStopped(bool ROOTFileOpen)
{
  AQStartStop_TB->SetBackgroundColor(ColorManager->Number2Pixel(ButtonBackColorOff));
  AQStartStop_TB->SetForegroundColor(ColorManager->Number2Pixel(kWhite));
  AQStartStop_TB->SetText("Stopped");
  
  SetAcquisitionWidgetState(true, kButtonUp);
  
  // Reset the attributes of the timer start text button
  AQTimerStart_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
  AQTimerStart_TB->SetForegroundColor(ColorManager->Number2Pixel(kBlack));
  AQTimerStart_TB->SetText("Start timer");

  Bool_t ADAQFileIsOpen = AAAcquisitionManager::GetInstance()->GetADAQFileIsOpen();

  // If an ADAQ file is open then the storage widget check boxes
  // (waveform, energy data, PSD data) and the waveform file name
  // setting button are disabled. We want to reenable them but ensure
  // that those that were checked remain checked. If an ADAQ file is
  // not open then these widgets do not need to be updated
  
  if(ADAQFileIsOpen){
    if(WaveformFileName_TB->GetState() == kButtonDisabled)
      WaveformFileName_TB->SetState(kButtonUp);
    
    if(WaveformStoreRaw_CB->IsDisabledAndSelected())
      WaveformStoreRaw_CB->SetState(kButtonDown);
    else
      WaveformStoreRaw_CB->SetState(kButtonUp);
    
    if(WaveformStoreEnergyData_CB->IsDisabledAndSelected())
      WaveformStoreEnergyData_CB->SetState(kButtonDown);
    else
      WaveformStoreEnergyData_CB->SetState(kButtonUp);
    
    if(WaveformStorePSDData_CB->IsDisabledAndSelected())
      WaveformStorePSDData_CB->SetState(kButtonDown);
    else
      WaveformStorePSDData_CB->SetState(kButtonUp);
  }
  
  WaveformCreateFile_TB->SetState(kButtonDisabled);
  WaveformCreateFile_TB->SetText("Create");
  WaveformCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
  WaveformCreateFile_TB->SetForegroundColor(ColorManager->Number2Pixel(kBlack));
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


void AAInterface::UpdateHVMonitors(int Channel, int HV, int I)
{
  HVChVoltageMonitor_NEFL[Channel]->GetEntry()->SetNumber(HV);
  HVChCurrentMonitor_NEFL[Channel]->GetEntry()->SetNumber(I);
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

      FileName = FileInformation.fFilename;

      // Search for the file name extensions '.adaq.root' (data
      // output) and 'cfg.root' (interface configuration). Note that
      // the '\0' character indicates a string termination)
      size_t ADAQFilePos = FileName.find(".adaq.root\0");
      size_t SettingsFilePos = FileName.find(".acq.root\0");
      
      // If the extension was entered then do nothing...
      if(ADAQFilePos != string::npos or SettingsFilePos != string::npos){
      }
      
      // 
      else{
	
	// If the user has entered a file extension, strip it and
	// re-add the proper extension to force correctness

	size_t Found = FileName.find_last_of(".");
	if(Found != string::npos)
	  FileName = FileName.substr(0, Found) + FileExt;
	
	// ... or if no file extension was entered then tack on
	// '.adaq.root' file extension
	else
	  FileName += FileExt;
      }
    }
  }
  return FileName;
}
