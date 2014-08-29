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


AATabSlots::AATabSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AATabSlots::~AATabSlots()
{;}


void AATabSlots::HandleConnectionTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  // TEMPORARY
  enum{V1718, V1720, V6534};

  // Temporarily redirect the std::cout messages to a local buffer
  streambuf* StandardBuffer = cout.rdbuf();
  ostringstream NewBuffer;
  cout.rdbuf( NewBuffer.rdbuf() );
  
  switch(TextButtonID){
    
    // Connect AAInterface with VME boards
  case VMEConnect_TB_ID:

    // If no connection is presently established...
    if(!TheVMEManager->GetVMEConnectionEstablished()){
      
      int DGLinkOpen = -42;
      if(TheVMEManager->GetDGEnable()){
	uint32_t Addr = TI->BoardAddress_NEF[V1720]->GetHexNumber();
	TheVMEManager->SetDGAddress(Addr);
	DGLinkOpen = TheVMEManager->GetDGManager()->OpenLink(Addr);
	
	if(DGLinkOpen == 0)
	  TheVMEManager->GetDGManager()->Initialize();
      }
      
      int BRLinkOpen = -42;
      if(TheVMEManager->GetBREnable()){
	
	// Connect to the VME/USB bridge WITHOUT a digitizer enabled ...
	if(!TheVMEManager->GetDGEnable())
	  BRLinkOpen = TheVMEManager->GetBRManager()->OpenLinkDirectly();

	// Connect to the VME/USB bridge VIA the digitizer
	else if(DGLinkOpen == 0)
	  BRLinkOpen = TheVMEManager->GetBRManager()->OpenLinkViaDigitizer(TheVMEManager->GetDGManager()->GetBoardHandle(), true);
      }
      
      int HVLinkOpen = -42;
      if(TheVMEManager->GetHVEnable()){
	uint32_t Addr = TI->BoardAddress_NEF[V6534]->GetHexNumber();
	TheVMEManager->SetHVAddress(Addr);
	HVLinkOpen = TheVMEManager->GetHVManager()->OpenLink(Addr);
	
	if(HVLinkOpen == 0)
	  TheVMEManager->GetHVManager()->SetToSafeState();
      }
      
      if(BRLinkOpen == 0 or DGLinkOpen == 0 or HVLinkOpen == 0){
	TheVMEManager->SetVMEConnectionEstablished(true);

	TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
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
	TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
    }

    // If a connection is already established then terminate the connection
    else if(TheVMEManager->GetVMEConnectionEstablished()){
      TheVMEManager->SetVMEConnectionEstablished(false);
      
      // Call the functions that accomplishes safe shutdown of the
      // V6534 and V1720 boards (powering down voltages, turning all
      // channels off, etc)
      TI->HandleDisconnectAndTerminate(false);
      
      // Change the color and text of the button.
      TI->VMEConnect_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
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
    break;

    // Set the V6534Enable boolean that controls whether or not the
    // V6534 high voltage board should be presently used

  case V6534BoardEnable_TB_ID:
    if(TI->BoardEnable_TB[V6534]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[V6534]->SetText("Board disabled");
      TI->BoardEnable_TB[V6534]->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      TheVMEManager->SetHVEnable(false);
    }
    else if(TI->BoardEnable_TB[V6534]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[V6534]->SetText("Board enabled");
      TI->BoardEnable_TB[V6534]->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      TheVMEManager->SetHVEnable(true);
    }
    break;

    // Set the V1720Enable boolean that controls whether or not the
    // V1720 high voltage board should be presently used
  case V1720BoardEnable_TB_ID:
    if(TI->BoardEnable_TB[V1720]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[V1720]->SetText("Board disabled");
      TI->BoardEnable_TB[V1720]->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      TheVMEManager->SetDGEnable(false);
    }
    else if(TI->BoardEnable_TB[V1720]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[V1720]->SetText("Board enabled");
      TI->BoardEnable_TB[V1720]->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      TheVMEManager->SetDGEnable(true);
    }
    break;

  case V1718BoardEnable_TB_ID:
    if(TI->BoardEnable_TB[V1718]->GetString() == "Board enabled"){
      TI->BoardEnable_TB[V1718]->SetText("Board disabled");
      TI->BoardEnable_TB[V1718]->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      TheVMEManager->SetBREnable(false);
    }
    else if(TI->BoardEnable_TB[V1718]->GetString() == "Board disabled"){
      TI->BoardEnable_TB[V1718]->SetText("Board enabled");
      TI->BoardEnable_TB[V1718]->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      TheVMEManager->SetBREnable(true);
    }
    break;
  }

  // Redirect std::cout back to the normal terminal output
  cout.rdbuf(StandardBuffer);
}


void AATabSlots::HandleRegisterTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

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
    Addr32 = TI->ReadAddress_NEF[Board]->GetHexNumber();
    
    if(Board == V1718 and TheVMEManager->GetBREnable())
      TheVMEManager->GetBRManager()->GetRegisterValue(Addr32, &Data32);

    else if(Board == V1720 and TheVMEManager->GetDGEnable())
      TheVMEManager->GetDGManager()->GetRegisterValue(Addr32, &Data32);

    else if(Board == V6534 and TheVMEManager->GetHVEnable()){
      TheVMEManager->GetHVManager()->GetRegisterValue(Addr32, &Data16);
      Data32 = Data16;
    }

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

    if(Board == V1718)
      TheVMEManager->GetBRManager()->SetRegisterValue(Addr32, Data32);

    if(Board == V1720 and TheVMEManager->GetDGEnable())
      TheVMEManager->GetDGManager()->SetRegisterValue(Addr32, Data32);
    
    else if(Board == V6534 and TheVMEManager->GetHVEnable())
      TheVMEManager->GetHVManager()->SetRegisterValue(Addr32, Data32);
  }
}


void AATabSlots::HandlePulserTextButtons()
{
  TGTextButton *TextButton = (TGTextButton *) gTQSender;
  int TextButtonID = TextButton->WidgetId();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  if(TheVMEManager->GetVMEConnectionEstablished())
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
    TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
    TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
    TextButton->SetText("Pulsing");

    TheVMEManager->GetBRManager()->StartPulser(Pulser);
  }
  else if(TextButton->GetString()=="Pulsing"){
    // Update button color from green to red and update text
    TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
    TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
    TextButton->SetText("Stopped");
    
    TheVMEManager->GetBRManager()->StopPulser(Pulser);
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
    
  case HVChannel0Power_TB_ID:
  case HVChannel1Power_TB_ID:
  case HVChannel2Power_TB_ID:
  case HVChannel3Power_TB_ID:
  case HVChannel4Power_TB_ID:
  case HVChannel5Power_TB_ID:{
    
    // Determine the HV channel number corresponding to the clicked button
    int HVChannel = TI->HVChannelPower_TB_ID_Map[TextButtonID];

    // If the power is being turned from "OFF" to "ON"...
    if(TextButton->GetString()=="OFF"){
      // Update the button color from red to green and set text to "ON"
      TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      TextButton->SetText("ON");
      
      // Get the voltage and maximum current settings from the ROOT
      // number entry widgets for the desired HV channel
      int HVVoltageValue = TI->HVChannelV_NEL[HVChannel]->GetEntry()->GetIntNumber();
      int HVCurrentValue = TI->HVChannelI_NEL[HVChannel]->GetEntry()->GetIntNumber();
      
      // Set the voltage and maxmimum current drawn and turn the HV channel on
      TheVMEManager->GetHVManager()->SetVoltage(HVChannel, HVVoltageValue); 
      TheVMEManager->GetHVManager()->SetCurrent(HVChannel, HVCurrentValue);
      TheVMEManager->GetHVManager()->SetPowerOn(HVChannel);
      
      TI->SetVoltageChannelWidgetState(HVChannel, true);
    }

    // If the power is being turned from "ON" to "OFF"...
    else if(TextButton->GetString()=="ON"){
      // Update the button color from green to red and set text to "OFF"
      TextButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      TextButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
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
	TI->HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(true);
	TI->HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(true);
      }

      // Run the HV monitoring
      //RunHVMonitoring();
    }

    // If monitoring is being turned off...
    else{

      // Disable the HV channel monitoring ROOT number entry field
      // widgets to show that monitoring is turned off
      for(int ch=0; ch<HVChannels; ch++){
	TI->HVChannelVMonitor_NEFL[ch]->GetEntry()->SetState(false);
	TI->HVChannelIMonitor_NEFL[ch]->GetEntry()->SetState(false);
      }

      // Set bool to disable HV monitoring loop
      //HVMonitorEnable = false;
      break;
    }
  }
  }
}
