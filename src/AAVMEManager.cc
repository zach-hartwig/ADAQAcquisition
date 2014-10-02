#include "ADAQBridge.hh"
#include "ADAQDigitizer.hh"
#include "ADAQHighVoltage.hh"

#include "AAVMEManager.hh"


AAVMEManager *AAVMEManager::TheVMEManager = 0;


AAVMEManager *AAVMEManager::GetInstance()
{ return TheVMEManager; }


AAVMEManager::AAVMEManager()
  : BREnable(true), DGEnable(true), HVEnable(false),
    DGAddress(0x00420000), HVAddress(0x42420000),
    VMEConnectionEstablished(false)
{
  if(TheVMEManager)
    cout << "\nError! The VMEManager was constructed twice!\n" << endl;
  TheVMEManager = this;

  BRMgr = new ADAQBridge;
  BRMgr->SetVerbose(true);

  DGMgr = new ADAQDigitizer;
  DGMgr->SetVerbose(true);

  HVMgr = new ADAQHighVoltage;
  HVMgr->SetVerbose(true);
}


AAVMEManager::~AAVMEManager()
{;}


void AAVMEManager::ProgramDigitizers()
{
  DGMgr->Reset();

  uint32_t DGNumChEnabled = 0;
  uint32_t DGChEnableMask = 0;

  ////////////////////////////
  // Channel-specific settings

  for(int ch=0; ch<DGMgr->GetNumChannels(); ch++){
    // Calculate the channel enable mask, where each hex digit
    // represents channel state as "0" == disabled, "1" == enabled
    if(TheSettings->ChEnable[ch]){
      uint32_t Ch = 0x00000001<<ch;
      DGChEnableMask |= Ch;
      DGNumChEnabled++;
    }
    
    DGMgr->SetChannelDCOffset(ch, TheSettings->ChDCOffset[ch]);
    DGMgr->SetChannelTriggerThreshold(ch, TheSettings->ChTriggerThreshold[ch]);
    
    DGMgr->SetZLEChannelSettings(ch,
				 TheSettings->ChZSThreshold[ch],
				 TheSettings->ChZSBackward[ch],
				 TheSettings->ChZSForward[ch],
				 TheSettings->ChZSPosLogic[ch]);
  }

  DGMgr->SetChannelEnableMask(DGChEnableMask);

  // Ensure that at least one channel is enabled in the channel
  // enabled bit mask; if not, alert the user and return without
  // beginning acquisition since there ain't nothin' to acquire.


  ///////////////////
  // Trigger settings

  // Trigger type

  switch(TheSettings->TriggerType){

  case 0: // External (NIM logic)
    DGMgr->EnableExternalTrigger("NIM");
    DGMgr->DisableAutoTrigger(DGChEnableMask);
    DGMgr->DisableSWTrigger();
    break;

  case 1: // External (TTL logic)
    DGMgr->EnableExternalTrigger("TTL");
    DGMgr->DisableAutoTrigger(DGChEnableMask);
    DGMgr->DisableSWTrigger();
    break;
    
  case 2: // Automatic
    DGMgr->DisableExternalTrigger();
    DGMgr->EnableAutoTrigger(DGChEnableMask);
    DGMgr->DisableSWTrigger();
    break;
    
  case 3: // Software
    DGMgr->DisableExternalTrigger();
    DGMgr->DisableAutoTrigger(DGChEnableMask);
    DGMgr->EnableSWTrigger();
    break;

  default:
    break;
  }

  // Trigger edge (channel-specific but treated as group setting)
  
  for(int ch=0; ch<DGMgr->GetNumChannels(); ch++){

    switch(TheSettings->TriggerEdge){
      
    case 0: // Rising edge
      DGMgr->SetTriggerEdge(ch, "Rising");
      break;
      
    case 1: // Falling edge
      DGMgr->SetTriggerEdge(ch, "Falling");
      break;
      
    default:
      break;
    }
  }
  
  if(TheSettings->TriggerCoincidenceEnable and
     TheSettings->TriggerCoincidenceLevel < DGNumChEnabled)
    DGMgr->SetTriggerCoincidence(true, TheSettings->TriggerCoincidenceLevel);


  ///////////////////////
  // Acquisition settings

  DGMgr->SetRecordLength(TheSettings->RecordLength);
  DGMgr->SetPostTriggerSize(TheSettings->PostTrigger);


  ///////////////////
  // Readout settings

  DGMgr->SetMaxNumEventsBLT(TheSettings->EventsBeforeReadout);

  if(TheSettings->ZeroSuppressionEnable)
    DGMgr->SetZSMode("ZLE");
  else
    DGMgr->SetZSMode("None");
}


void AAVMEManager::SafelyDisconnectVMEBoards()
{
  if(HVEnable){
    HVMgr->SetToSafeState();
    HVMgr->CloseLink();
  }
  
  if(DGEnable)
    DGMgr->CloseLink();
  
  if(BREnable)
    BRMgr->CloseLink();
}



// Run the real-time updating of the ROOT number entry widgets that
// display active voltage and drawn current from all channels
//void AAInterface::RunHVMonitoring()
//{
  /*
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
    const int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    for(int ch=0; ch<HVChannels; ch++){
      // Get the present active voltage and current values
      TheVMEManager->GetHVManager()->GetVoltage(ch, &Voltage);
      TheVMEManager->GetHVManager()->GetCurrent(ch, &Current);
      
      // Update the appropriate number entry fields
      HVChannelVMonitor_NEFL[ch]->GetEntry()->SetNumber(Voltage);
      HVChannelIMonitor_NEFL[ch]->GetEntry()->SetNumber(Current);
    }
  }
  */
//}
