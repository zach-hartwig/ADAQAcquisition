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

// Boost
#include <boost/cstdint.hpp>

// C++
#include <iostream>
#include <sstream>
#include <bitset>

// ADAQ
#include "ADAQBridge.hh"
#include "ADAQDigitizer.hh"
#include "ADAQHighVoltage.hh"

// ADAQAcquisition
#include "AATabSlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"


AATabSlots::AATabSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AATabSlots::~AATabSlots()
{;}


void AATabSlots::HandleSettingsTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  switch(TextButtonID){
    
  case SetSettingsFileName_TB_ID:{
    
    const char *FileTypes[] = {"Settings file", "*.acq.root",
			       0, 0};
    
    string SettingsFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(SettingsFileName == "NULL"){
    }
    else{
      string FileNameNoPath = SettingsFileName;
      
      size_t Found = FileNameNoPath.find_last_of("/");
      if(Found != string::npos)
	FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
      
      TI->SettingsFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
      TI->SettingsFileName = SettingsFileName;

      TI->SaveSettingsToFile_TB->SetText("Save");
      TI->SaveSettingsToFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
      TI->SaveSettingsToFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));

      TI->LoadSettingsFromFile_TB->SetText("Load");
      TI->LoadSettingsFromFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
      TI->LoadSettingsFromFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
    }
  }
    break;
    
  case SaveSettingsToFile_TB_ID:
    TI->SaveSettingsToFile();
    TI->SaveSettingsToFile_TB->SetText("Saved!");
    TI->SaveSettingsToFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kWhite));
    TI->SaveSettingsToFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
    break;

  case LoadSettingsFromFile_TB_ID:
    TI->LoadSettingsFromFile();
    TI->LoadSettingsFromFile_TB->SetText("Loaded!");
    TI->LoadSettingsFromFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kWhite));
    TI->LoadSettingsFromFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
    break;
    
  default:
    break;
  }
}

void AATabSlots::HandleConnectionTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  enum{BR, DG, HV};
  
  switch(TextButtonID){
    
    // Connect AAInterface with VME boards
  case VMEConnect_TB_ID:{

    // Temporarily redirect the std::cout messages to a local buffer
    streambuf *StandardBuffer = cout.rdbuf();
    ostringstream NewBuffer;
    cout.rdbuf( NewBuffer.rdbuf() );

    // If no connection is presently established...
    if(!TheVMEManager->GetVMEConnectionEstablished()){

      Int_t DGLinkOpen = -42;
      if(TI->BoardEnable_TB[DG]->GetString() == "Board enabled"){
	
	uint32_t Addr = TI->BoardAddress_NEF[DG]->GetEntry()->GetHexNumber();
	int Link = TI->BoardLinkNumber_NEL[DG]->GetEntry()->GetIntNumber();
	ZBoardType Type = (ZBoardType)TI->BoardType_CBL[DG]->GetComboBox()->GetSelected();
	
	TheVMEManager->SetDGEnable(true);
	TheVMEManager->SetDGType(Type);
	TheVMEManager->SetDGAddress(Addr);
	TheVMEManager->SetDGLinkNumber(Link);
	
	DGLinkOpen = TheVMEManager->InitializeDigitizer();
	
	if(DGLinkOpen == 0)
	  TheVMEManager->GetDGManager()->Initialize();
      }
      
      int BRLinkOpen = -42;
      if(TI->BoardEnable_TB[BR]->GetString() == "Board enabled"){
	
	ZBoardType Type = (ZBoardType)TI->BoardType_CBL[BR]->GetComboBox()->GetSelected();
	
	TheVMEManager->SetBREnable(true);
	TheVMEManager->SetBRType(Type);
	
	BRLinkOpen = TheVMEManager->InitializeBridge();
      }
      
      int HVLinkOpen = -42;
      if(TI->BoardEnable_TB[HV]->GetString() == "Board enabled"){
	
	uint32_t Addr = TI->BoardAddress_NEF[HV]->GetEntry()->GetHexNumber();
	int Link = TI->BoardLinkNumber_NEL[HV]->GetEntry()->GetIntNumber();
	ZBoardType Type = (ZBoardType)TI->BoardType_CBL[HV]->GetComboBox()->GetSelected();
	
	TheVMEManager->SetHVEnable(true);
	TheVMEManager->SetHVType(Type);
	TheVMEManager->SetHVAddress(Addr);
	TheVMEManager->SetHVLinkNumber(Link);
	
	HVLinkOpen = TheVMEManager->InitializeHighVoltage();
	
	if(HVLinkOpen == 0)
	  TheVMEManager->GetHVManager()->SetToSafeState();
      }
      
      if(BRLinkOpen == 0 or DGLinkOpen == 0 or HVLinkOpen == 0){

	TheVMEManager->SetVMEConnectionEstablished(true);

	TI->BuildSecondaryFrames();

	AAAcquisitionManager::GetInstance()->Initialize();

	TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
	TI->VMEConnect_TB->SetText("Connected: click to disconnect");
	
	// Convert the new "std::cout" buffer into a TGText
	string InputString = NewBuffer.str();
	TGText *InputText = new TGText;
	InputText->LoadBuffer(InputString.c_str());
	
	// Update the connection TGTextView with the status messages
	TI->ConnectionOutput_TV->AddText(InputText);
	TI->ConnectionOutput_TV->ShowBottom();
	TI->ConnectionOutput_TV->Update();
      }
      else
	TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
    }
    
    // If a connection is already established then terminate the connection
    else if(TheVMEManager->GetVMEConnectionEstablished()){
      
      TheVMEManager->SetVMEConnectionEstablished(false);

      TI->HandleDisconnectAndTerminate(false);

      // Change the color and text of the button.
      TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TI->VMEConnect_TB->SetText("Disconnected: click to connect");

      // Convert the new "std::cout" buffer into a TGText
      string InputString = NewBuffer.str();
      TGText *InputText = new TGText;
      InputText->LoadBuffer(InputString.c_str());
      
      // Update the connection TGTextView with the status messages
      TI->ConnectionOutput_TV->AddText(InputText);
      TI->ConnectionOutput_TV->ShowBottom();
      TI->ConnectionOutput_TV->Update();
    }

    // Redirect std::cout back to the normal terminal output
    cout.rdbuf(StandardBuffer);
    break;
  }

    // Set the V6534Enable boolean that controls whether or not the
    // V6534 high voltage board should be presently used

  case HVBoardEnable_TB_ID:
    if(TI->BoardEnable_TB[HV]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[HV]->SetText("Board disabled");
      TI->BoardEnable_TB[HV]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TI->VoltageTab->HideFrame(TI->VoltageFrame);
      TheVMEManager->SetHVEnable(false);
    }
    else if(TI->BoardEnable_TB[HV]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[HV]->SetText("Board enabled");
      TI->BoardEnable_TB[HV]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TI->VoltageTab->ShowFrame(TI->VoltageFrame);
      TheVMEManager->SetHVEnable(true);
    }
    break;
    
    // Set the V1720Enable boolean that controls whether or not the
    // V1720 high voltage board should be presently used
  case DGBoardEnable_TB_ID:
    if(TI->BoardEnable_TB[DG]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[DG]->SetText("Board disabled");
      TI->BoardEnable_TB[DG]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TI->AcquisitionTab->HideFrame(TI->AcquisitionFrame);
      TheVMEManager->SetDGEnable(false);
    }
    else if(TI->BoardEnable_TB[DG]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[DG]->SetText("Board enabled");
      TI->BoardEnable_TB[DG]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TI->AcquisitionTab->ShowFrame(TI->AcquisitionFrame);
      TheVMEManager->SetDGEnable(true);
    }
    break;

  case DGCalibrateADCs_TB_ID:
    if(TheVMEManager->GetDGLinkOpen())
      TheVMEManager->GetDGManager()->Calibrate();
    break;
    
  case BRBoardEnable_TB_ID:
    if(TI->BoardEnable_TB[BR]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[BR]->SetText("Board disabled");
      TI->BoardEnable_TB[BR]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TI->PulserTab->HideFrame(TI->PulserFrame);
      TheVMEManager->SetBREnable(false);
    }
    else if(TI->BoardEnable_TB[BR]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[BR]->SetText("Board enabled");
      TI->BoardEnable_TB[BR]->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TI->PulserTab->ShowFrame(TI->PulserFrame);
      TheVMEManager->SetBREnable(true);
    }
    break;
  }
}


void AATabSlots::HandlePulserTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  TI->SaveSettings();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  if(!TheVMEManager->GetVMEConnectionEstablished() or
     !TheVMEManager->GetBREnable())
    return;
  
  enum {PulserA, PulserB};
  int Pulser = -1;

  switch(TextButtonID){
    
  case V1718PulserA_TB_ID:
    Pulser = PulserA;
    break;
    
  case V1718PulserB_TB_ID:
    Pulser = PulserB;
    break;
  }

  PulserSettings PS;
  PS.PulserToSet = Pulser;
  PS.Period = TI->V1718PulserPeriod_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.Width = TI->V1718PulserWidth_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.TimeUnit = TI->V1718PulserTimeUnit_CBL[Pulser]->GetComboBox()->GetSelected();
  PS.PulseNumber = TI->V1718PulserPulses_NEL[Pulser]->GetEntry()->GetIntNumber();
  PS.StartSource = TI->V1718PulserStartSource_CBL[Pulser]->GetComboBox()->GetSelected();
  PS.StopSource = TI->V1718PulserStopSource_CBL[Pulser]->GetComboBox()->GetSelected();

  PulserOutputSettings POS;
  POS.OutputLine = TI->V1718PulserOutputLine_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.OutputPolarity = TI->V1718PulserOutputPolarity_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.LEDPolarity = TI->V1718PulserLEDPolarity_CBL[Pulser]->GetComboBox()->GetSelected();
  POS.Source = TI->V1718PulserSource_CBL[Pulser]->GetComboBox()->GetSelected();

  TheVMEManager->GetBRManager()->SetPulserSettings(&PS);
  TheVMEManager->GetBRManager()->SetPulserOutputSettings(&POS);

  if(TextButton->GetString()=="Stopped"){
    // Update button color from red to green andn update text
    TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
    TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    TextButton->SetText("Pulsing");

    TheVMEManager->GetBRManager()->StartPulser(Pulser);
  }
  else if(TextButton->GetString()=="Pulsing"){
    // Update button color from green to red and update text
    TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
    TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    TextButton->SetText("Stopped");
    
    TheVMEManager->GetBRManager()->StopPulser(Pulser);
  }

}


void AATabSlots::HandleRegisterTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;

  // 32-bit integers to hold register addresses and data; 16-bit
  // integer to hold data obtained from the V6534 board
  uint32_t Addr32 = 0;
  uint32_t Data32 = 0;
  uint16_t Data16 = 0;

  // Create two enums for local parsing of the actions
  enum{READ,WRITE};

  // TEMPORARY
  enum{V1718, V1720, V6534};

  // Use a case statement to two select the action and the board upon
  // which the register read/write will take place
  int Action = 0;
  int Board = 0;
  switch(TextButtonID){
    
  case BRRead_ID:
    Action = READ;
    Board = V1718;
    break;

  case BRWrite_ID:
    Action = WRITE;
    Board = V1718;
    break;

  case DGRead_ID:
    Action = READ;
    Board = V1720;
    break;

  case DGWrite_ID:
    Action = WRITE;
    Board = V1720;
    break;
    
  case HVRead_ID:
    Action = READ;
    Board = V6534;
    break;

  case HVWrite_ID:
    Action = WRITE;
    Board = V6534;
    break;
  }

  ///////////////////////////////////////////////
  // Perform a register read of the desired board

  if(Action == READ){
    Addr32 = TI->ReadAddress_NEF[Board]->GetHexNumber();
    
    if(Board == V1718 and TheVMEManager->GetBREnable())
      TheVMEManager->GetBRManager()->GetRegisterValue(Addr32, &Data32);

    else if(Board == V1720 and TheVMEManager->GetDGEnable())
      TheVMEManager->GetDGManager()->GetRegisterValue(Addr32, &Data32);

    else if(Board == V6534 and TheVMEManager->GetHVEnable()){
      TheVMEManager->GetHVManager()->GetRegisterValue(Addr32, &Data16);
      Data32 = Data16;
    }

    // Update the dec widget with the register value
    TI->ReadValueDec_NEF[Board]->SetIntNumber(Data32);
    
    // Update the hex widget with the register value
    TI->ReadValueHex_NEF[Board]->SetHexNumber(Data32);
    
    // Update the binary widget with the register value inserting
    // spaces every 4 characters for easier reading
    bitset<32> Value(Data32);
    string ValueString = Value.to_string();
    int offset = 0;
    for(int i=0; i<32/4; i++){
      ValueString.insert(i*4+offset, " ");
      offset++;
    }
    TI->ReadValueBinary_TE[Board]->SetText(ValueString.c_str());
  }

  ////////////////////////////////////////////////
  // Perform a register write of the desired board
  else if(Action == WRITE){
    Addr32 = TI->WriteAddress_NEF[Board]->GetHexNumber();
    Data32 = TI->WriteValue_NEF[Board]->GetHexNumber();
    
    if(Board == V1718 and TheVMEManager->GetBREnable())
      TheVMEManager->GetBRManager()->SetRegisterValue(Addr32, Data32);
    
    if(Board == V1720 and TheVMEManager->GetDGEnable())
      TheVMEManager->GetDGManager()->SetRegisterValue(Addr32, Data32);
    
    else if(Board == V6534 and TheVMEManager->GetHVEnable())
      TheVMEManager->GetHVManager()->SetRegisterValue(Addr32, Data32);
  }
}


void AATabSlots::HandleVoltageTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  if(!TheVMEManager->GetHVEnable())
    return;

  switch(TextButtonID){
    
    ///////////////////////
    // HV Power Settings 
    
  case HVCh0Power_TB_ID:
  case HVCh1Power_TB_ID:
  case HVCh2Power_TB_ID:
  case HVCh3Power_TB_ID:
  case HVCh4Power_TB_ID:
  case HVCh5Power_TB_ID:{
    
    // Determine the HV channel number corresponding to the clicked button
    int HVChannel = TI->HVChPower_TB_ID_Map[TextButtonID];

    // If the power is being turned from "OFF" to "ON"...
    if(TextButton->GetString()=="OFF"){
      // Update the button color from red to green and set text to "ON"
      TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TextButton->SetText("ON");
      
      // Get the voltage and maximum current settings from the ROOT
      // number entry widgets for the desired HV channel
      int HVVoltageValue = TI->HVChVoltage_NEL[HVChannel]->GetEntry()->GetIntNumber();
      int HVCurrentValue = TI->HVChCurrent_NEL[HVChannel]->GetEntry()->GetIntNumber();

      // Set the voltage and maxmimum current drawn and turn the HV channel on
      TheVMEManager->GetHVManager()->SetVoltage(HVChannel, HVVoltageValue); 
      TheVMEManager->GetHVManager()->SetCurrent(HVChannel, HVCurrentValue);
      TheVMEManager->GetHVManager()->SetPowerOn(HVChannel);
      
      TI->SetVoltageChannelWidgetState(HVChannel, true);
    }

    // If the power is being turned from "ON" to "OFF"...
    else if(TextButton->GetString()=="ON"){
      // Update the button color from green to red and set text to "OFF"
      TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TextButton->SetText("OFF");

      // Turn the HV channel off
      TheVMEManager->GetHVManager()->SetPowerOff(HVChannel);
      
      // Reenable the widget status such that voltage and maximum
      // current can be modified
      TI->SetVoltageChannelWidgetState(HVChannel, false);
    }
    break;
  }
    
    //////////////////
    // HV Monitoring 
    
    // Enable real-time monitoring of all channel's voltage and drawn current
  case HVEnableMonitoring_CB_ID:{

    const int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    
    // If monitoring is being turned on....
    if(TI->HVMonitorEnable_CB->IsDown()){
      
      // Set bool to enable the HV monitoring loop (see
      // AAInterface::RunHVMonitoring)
      //TI->HVMonitorEnable = true;

      // Enable the HV channel monitoring ROOT number entry field
      // widgets to show that monitoring is turned on
      for(int ch=0; ch<HVChannels; ch++){
	TI->HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(true);
	TI->HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(true);
      }

      // Run the HV monitoring
      //RunHVMonitoring();
    }

    // If monitoring is being turned off...
    else{

      // Disable the HV channel monitoring ROOT number entry field
      // widgets to show that monitoring is turned off
      for(int ch=0; ch<HVChannels; ch++){
	TI->HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(false);
	TI->HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(false);
      }

      // Set bool to disable HV monitoring loop
      //HVMonitorEnable = false;
      break;
    }
  }
  }
}


void AATabSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  switch(ActiveID){
    
  case HVEnableMonitoring_CB_ID:{
    
    int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    
    if(ActiveButton->IsDown()){
      for(int ch=0; ch<HVChannels; ch++){
	TI->HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(true);
	TI->HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(true);
      }
      TheVMEManager->StartHVMonitoring(TI);
    }
    else{
      for(int ch=0; ch<HVChannels; ch++){
	TI->HVChVoltageMonitor_NEFL[ch]->GetEntry()->SetState(false);
	TI->HVChCurrentMonitor_NEFL[ch]->GetEntry()->SetState(false);
      }
      TheVMEManager->StopHVMonitoring();
      break;
    }
  }
  }
}


void AATabSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  // Do not save the interface state for these buttons. Saving of
  // these values will be done upon clicking the VME connection text
  // button in order to allow these buttons to be clicked *before*
  // dynamically building the rest of the GUI widgets
  //
  // TI->SaveSettings();
  
  switch(ActiveID){
  case DGStandardFW_RB_ID:
    if(TI->DGStandardFW_RB->IsDown())
      TI->DGPSDFW_RB->SetState(kButtonUp);
    break;

  case DGPSDFW_RB_ID:
    if(TI->DGPSDFW_RB->IsDown())
      TI->DGStandardFW_RB->SetState(kButtonUp);
    break;

  default:
    break;
  }
}


