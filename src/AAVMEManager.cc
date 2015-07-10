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

#include <TSystem.h>

#include "ADAQBridge.hh"
#include "ADAQDigitizer.hh"
#include "ADAQHighVoltage.hh"

#include "AAVMEManager.hh"


AAVMEManager *AAVMEManager::TheVMEManager = 0;


AAVMEManager *AAVMEManager::GetInstance()
{ return TheVMEManager; }


AAVMEManager::AAVMEManager()
  : BREnable(true), DGEnable(true), HVEnable(false),
    BRIdentifier(0),
    DGIdentifier(0), DGAddress(0x00420000), DGLinkNumber(0), DGCONETNode(0),
    HVIdentifier(0), HVAddress(0x42420000), HVLinkNumber(0), 
    VMEConnectionEstablished(false)
{
  if(TheVMEManager)
    cout << "\nError! The VMEManager was constructed twice!\n" << endl;
  TheVMEManager = this;
  
  BRMgr = new ADAQBridge(zV1718, 
			 BRIdentifier);
  BRMgr->SetVerbose(true);
  
  DGMgr = new ADAQDigitizer(zV1720, 
			    DGIdentifier, 
			    DGAddress,
			    DGLinkNumber, 
			    DGCONETNode);
  DGMgr->SetVerbose(true);
  
  HVMgr = new ADAQHighVoltage(zV6534M, 
			      HVIdentifier,
			      HVAddress,
			      HVLinkNumber);

  HVMgr->SetVerbose(true);
}


AAVMEManager::~AAVMEManager()
{;}


bool AAVMEManager::ProgramDigitizers()
{
  //DGMgr->Reset();

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
    else
      continue;
    
    DGMgr->SetChannelDCOffset(ch, TheSettings->ChDCOffset[ch]);
    DGMgr->SetChannelTriggerThreshold(ch, TheSettings->ChTriggerThreshold[ch]);
    
    if(TheSettings->ChPosPolarity[ch])
      DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityPositive);
    else
      DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityNegative);
    
    if(TheSettings->ZeroSuppressionEnable){
      DGMgr->SetZSMode("ZLE");
      
      DGMgr->SetZLEChannelSettings(ch,
				   TheSettings->ChZLEThreshold[ch],
				   TheSettings->ChZLEBackward[ch],
				   TheSettings->ChZLEForward[ch],
				   TheSettings->ChZLEPosLogic[ch]);

      // Testing for positive ZLE logic
      if(TheSettings->ChZLEPosLogic[ch]){
	if(TheSettings->ChZLEThreshold[ch] > TheSettings->ChTriggerThreshold[ch]){
	  
	  cout << "Error! For ZLE positive logic: ZLE Threshold < Ch Trigger!\n"
	       << endl;
	  
	  return false;
	}
      }
      
      // Testing for negative ZLE logic
      else if(TheSettings->ChZLENegLogic[ch]){
	if(TheSettings->ChZLEThreshold[ch] < TheSettings->ChTriggerThreshold[ch]){
	  
	  cout << "Error! For ZLE negative logic: ZLE Threshold > Ch Trigger!\n"
	       << endl;
	  return false;
	}
      }
    }
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

  switch(TheSettings->AcquisitionControl){
    
  case 0: // Standard (software controlled)
    DGMgr->SetAcquisitionControl("Software");
    break;
    
  case 1: // Gated (NIM signal on S-IN Lemo 00 front panel)
    DGMgr->SetAcquisitionControl("Gated (NIM)");
    break;
    
  case 2: // Gated (TTL signal on S-IN Lemo 00 front panel)
    DGMgr->SetAcquisitionControl("Gated (TTL)");
    break;
    
  default:
    break;
  }
  
  DGMgr->SetRecordLength(TheSettings->RecordLength);
  DGMgr->SetPostTriggerSize(TheSettings->PostTrigger);
  
  ///////////////////
  // Readout settings
  
  DGMgr->SetMaxNumEventsBLT(TheSettings->EventsBeforeReadout);

  if(TheSettings->ZeroSuppressionEnable)
    DGMgr->SetZSMode("ZLE");
  else
    DGMgr->SetZSMode("None");

  return true;
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


void AAVMEManager::StartHVMonitoring(AAInterface *TI)
{
  HVMonitorEnable = true;

  //  vector<uint16_t> Voltage(HVMgr->GetNumChannels(), 0);
  //  vector<uint16_t> Current(HVMgr->GetNumChannels(), 0);
  
  // The high voltage and current will be displayed and updated in the
  // dedicated number entry fields when HVMonitorEnable is true
  while(HVMonitorEnable){

    // Perform action in a separate thread to enable use of other GUI
    // features while HV monitoring is taking place
    gSystem->ProcessEvents();
    
    // Update the voltage and current displays every half-second
    double delay = clock()+(0.5*CLOCKS_PER_SEC);
    while(clock()<delay){gSystem->ProcessEvents();}
    
    uint16_t Voltage, Current;
    const int HVChannels = TheVMEManager->GetHVManager()->GetNumChannels();
    for(int ch=0; ch<HVChannels; ch++){
      
      HVMgr->GetVoltage(ch, &Voltage);
      HVMgr->GetCurrent(ch, &Current);
      
      TI->UpdateHVMonitors(ch, Voltage, Current);
    }
  }
}


void AAVMEManager::StopHVMonitoring()
{ HVMonitorEnable = false; }
