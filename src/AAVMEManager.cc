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

typedef struct
{
  CAEN_DGTZ_ConnectionType LinkType;
  uint32_t VMEBaseAddress;
  uint32_t RecordLength;
  uint32_t ChannelMask;
  int EventAggr;
  CAEN_DGTZ_PulsePolarity_t PulsePolarity;
  CAEN_DGTZ_DPP_AcqMode_t AcqMode;
  CAEN_DGTZ_IOLevel_t IOlev;
} DigitizerParams_t;


AAVMEManager *AAVMEManager::TheVMEManager = 0;


AAVMEManager *AAVMEManager::GetInstance()
{ return TheVMEManager; }


AAVMEManager::AAVMEManager()
  : BREnable(false), BRType(0), BRIdentifier(0), BRLinkOpen(false),
    DGEnable(false), DGIdentifier(0), DGAddress(0x00000000),
    DGLinkNumber(0), DGCONETNode(0), DGLinkOpen(false),
    HVEnable(false), HVIdentifier(0), HVAddress(0x00000000),
    HVLinkNumber(0), HVLinkOpen(false),
    VMEConnectionEstablished(false)
{
  if(TheVMEManager)
    cout << "\nError! The VMEManager was constructed twice!\n" << endl;
  TheVMEManager = this;
}


AAVMEManager::~AAVMEManager()
{;}


Int_t AAVMEManager::InitializeBridge()
{
  BRMgr = new ADAQBridge((ZBoardType)BRType,
			 BRIdentifier);
  
  BRMgr->SetVerbose(true);
  
  Int_t Status = -42;
  
  // Connect to the VME/USB bridge WITHOUT a digitizer enabled ...
  if(!TheVMEManager->GetDGEnable())
    Status = BRMgr->OpenLinkDirectly();
  
  // Connect to the VME/USB bridge VIA the digitizer
  else if(DGEnable == 0)
    Status = BRMgr->OpenLinkViaDigitizer(DGMgr->GetBoardHandle(),
					 true);

  if(BRMgr->GetLinkEstablished())
    BRLinkOpen = true;
  
  return Status;
}


Int_t AAVMEManager::InitializeDigitizer()
{
  DGMgr = new ADAQDigitizer((ZBoardType)DGType,
			    DGIdentifier, 
			    DGAddress,
			    DGLinkNumber, 
			    DGCONETNode);
  
  DGMgr->SetVerbose(true);
  
  Int_t Status = DGMgr->OpenLink();

  if(DGMgr->GetLinkEstablished())
    DGLinkOpen = true;
  
  return Status;
}


Int_t AAVMEManager::InitializeHighVoltage()
{
  HVMgr = new ADAQHighVoltage((ZBoardType)HVType,
			      HVIdentifier,
			      HVAddress,
			      HVLinkNumber);
  
  
  HVMgr->SetVerbose(true);

  Int_t Status = -42;
  
  if(HVType == zDT5790M or HVType == zDT5790N or HVType == zDT5790P){

    if(!DGMgr->GetLinkEstablished())
      return Status;
    else{
      Int_t DGHandle = DGMgr->GetBoardHandle();
      HVMgr->SetBoardHandle(DGHandle);
    }
  }
  
  Status = HVMgr->OpenLink();

  if(HVMgr->GetLinkEstablished())
    HVLinkOpen = true;
  
  return Status;
}


bool AAVMEManager::ProgramDigitizers()
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

  if(TheSettings->STDFirmware){
    DGMgr->SetRecordLength(TheSettings->RecordLength);
    DGMgr->SetPostTriggerSize(TheSettings->PostTrigger);
    
    if(TheSettings->ZeroSuppressionEnable)
      DGMgr->SetZSMode("ZLE");
    else
      DGMgr->SetZSMode("None");
  }
  
  ///////////////////
  // Readout settings
  
  DGMgr->SetMaxNumEventsBLT(TheSettings->EventsBeforeReadout);

  
  if(TheSettings->PSDFirmware){
    
    // Create the mandatory DPP-PSD parameter struct
    CAEN_DGTZ_DPP_PSD_Params_t PSDParameters;
    
    for(Int_t ch=0; ch<DGMgr->GetNumChannels(); ch++){

      if(TheSettings->ChPosPolarity[ch])
	DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityPositive);
      else if(TheSettings->ChNegPolarity[ch])
	DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityNegative);
      
      DGMgr->SetChannelDCOffset(ch, TheSettings->ChDCOffset[ch]);
      DGMgr->SetRecordLength(TheSettings->ChRecordLength[ch], ch);
      PSDParameters.nsbl[ch] = TheSettings->ChBaselineSamples[ch];
      PSDParameters.csens[ch] = TheSettings->ChChargeSensitivity[ch];

      DGMgr->SetDPPPreTriggerSize(ch, 100);
      PSDParameters.selft[ch] = 1;
      PSDParameters.thr[ch] = TheSettings->ChTriggerThreshold[ch];
      PSDParameters.tvaw[ch] = TheSettings->ChTriggerValidation[ch];
      PSDParameters.trgc[ch] = (CAEN_DGTZ_DPP_TriggerConfig_t)TheSettings->ChTriggerConfig[ch];

      PSDParameters.sgate[ch] = TheSettings->ChShortGate[ch];
      PSDParameters.lgate[ch] = TheSettings->ChLongGate[ch];
      PSDParameters.pgate[ch] = TheSettings->ChPreGate[ch];
    }

    PSDParameters.purh = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
    PSDParameters.purgap = 100;  // Purity Gap
    PSDParameters.blthr = 3;     // Baseline Threshold
    PSDParameters.bltmo = 100;   // Baseline Timeout
    PSDParameters.trgho = 8;     // Trigger HoldOff
    
    DGMgr->SetDPPAcquisitionMode((CAEN_DGTZ_DPP_AcqMode_t)TheSettings->PSDOperationMode,
				 CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);
    
    DGMgr->SetIOLevel(CAEN_DGTZ_IOLevel_TTL);
    
    DGMgr->SetDPPEventAggregation(TheSettings->EventsBeforeReadout, 0);
    
    DGMgr->SetDPPParameters(DGChEnableMask, &PSDParameters);
    
    cout << "Finished PSD programming..." << endl;
  }
  
  return true;
}


void AAVMEManager::SafelyDisconnectVMEBoards()
{
  if(HVLinkOpen){
    
    HVMgr->SetToSafeState();
    
    if(HVType == zDT5790M or HVType == zDT5790N or HVType == zDT5790P){
    }
    else
      HVMgr->CloseLink();

    HVLinkOpen = false;
  }
  
  if(DGLinkOpen){
    DGMgr->CloseLink();
    DGLinkOpen = false;
  }
  
  if(BRLinkOpen){
    BRMgr->CloseLink();
    BRLinkOpen = false;
  }
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
