/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                           Copyright (C) 2012-2016                           //
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

  DGChEnableMask = DGMgr->CalculateChannelEnableMask(TheSettings->ChEnable);

  for(int ch=0; ch<DGMgr->GetNumChannels(); ch++){

    /*
    // Calculate the channel enable mask, where each hex digit
    // represents channel state as "0" == disabled, "1" == enabled
    if(TheSettings->ChEnable[ch]){
      uint32_t Ch = 0x00000001<<ch;
      DGChEnableMask |= Ch;
      DGNumChEnabled++;
    }
    else
      continue;
    */

    if(TheSettings->STDFirmware){
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
  }

  // Set the channel-enable mask
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

  if(TheSettings->STDFirmware){
  
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
  
  if(TheSettings->STDFirmware)
    DGMgr->SetMaxNumEventsBLT(TheSettings->EventsBeforeReadout);
  
  if(TheSettings->PSDFirmware){

    //////////////////////////////////////////////
    // Create and fill the PSD parameter structure

    // Note that many DPP-PSD devices (e.g. the 16 channel V1725 and
    // V1730 boards) use a "dual channel" approach for some settings,
    // i.e. setting Ch0 record length will actually set Ch0 and
    // Ch1. At present, DPP-PSD is under heavy development and
    // documentation is spotty at best. Future updates will try to
    // make this as clear to the user in the interface.
    
    CAEN_DGTZ_DPP_PSD_Params_t PSDParameters;
    
    for(Int_t ch=0; ch<DGMgr->GetNumChannels(); ch++){
      
      PSDParameters.nsbl[ch] = TheSettings->ChBaselineSamples[ch];
      PSDParameters.csens[ch] = TheSettings->ChChargeSensitivity[ch];
      
      // Channel self-triggering (automatic)
      if(TheSettings->TriggerType == 2)
	PSDParameters.selft[ch] = 1;
      
      // Software or external triggering
      else
	PSDParameters.selft[ch] = 0;
      
      PSDParameters.thr[ch] = TheSettings->ChTriggerThreshold[ch];
      PSDParameters.tvaw[ch] = TheSettings->ChTriggerValidation[ch];
      PSDParameters.trgc[ch] = (CAEN_DGTZ_DPP_TriggerConfig_t)TheSettings->ChTriggerConfig[ch];
      
      PSDParameters.sgate[ch] = TheSettings->ChShortGate[ch];
      PSDParameters.lgate[ch] = TheSettings->ChLongGate[ch]; 
      PSDParameters.pgate[ch] = TheSettings->ChGateOffset[ch];
    }

    // The trigger holdoff setting *should* be channel-specific (and,
    // indeed, CAEN claims it is in the CAENDigitizer user manual),
    // but it is in fact an int16 defined in CAENDigitizerType.h from
    // the CAENDigitzer-2.6.7 library. ZSH (28 Sep 15)
    PSDParameters.trgho = TheSettings->PSDTriggerHoldoff; // Trigger holdoff

    // Pileup rejection settings. These settings are Unimplemented at
    // present but may be used in the future. ZSH (28 Sep 15)
    //
    // PSDParameters.purh = CAEN_DGTZ_DPP_PSD_PUR_DetectOnly;
    // PSDParameters.purgap = 100; 

    // It is unclear from CAEN documentation whether or not these
    // parameters are still in use. I have asked for
    // clarification. ZSH (28 Sep 15)
    //
    // PSDParameters.blthr = 3;     // Baseline threshold  (Depracated?)
    // PSDParameters.bltmo = 100;   // Baseline timeout  (Depracated?)

    DGMgr->SetDPPParameters(DGChEnableMask, &PSDParameters);


    ///////////////////////////////////////////////////////
    // Set channel-specific, non-PSD structure PSD settings
    
    for(Int_t ch=0; ch<DGMgr->GetNumChannels(); ch++){
      
      DGMgr->SetRecordLength(TheSettings->ChRecordLength[ch], ch);
      
      DGMgr->SetChannelDCOffset(ch, TheSettings->ChDCOffset[ch]);
      
      DGMgr->SetDPPPreTriggerSize(ch, TheSettings->ChPreTrigger[ch]);
      
      if(TheSettings->ChPosPolarity[ch])
	DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityPositive);
      else if(TheSettings->ChNegPolarity[ch])
	DGMgr->SetChannelPulsePolarity(ch, CAEN_DGTZ_PulsePolarityNegative);
    }

    
    ////////////////////////////////////////////
    // Set global non-PSD structure PSD settings

    DGMgr->SetDPPAcquisitionMode((CAEN_DGTZ_DPP_AcqMode_t)TheSettings->PSDOperationMode,
				 CAEN_DGTZ_DPP_SAVE_PARAM_EnergyAndTime);

    if(TheSettings->TriggerCoincidenceEnable)
      DGMgr->SetDPPTriggerMode(CAEN_DGTZ_DPP_TriggerMode_Coincidence);
    else
      DGMgr->SetDPPTriggerMode(CAEN_DGTZ_DPP_TriggerMode_Normal);
    
    DGMgr->SetIOLevel(CAEN_DGTZ_IOLevel_TTL);
    
    DGMgr->SetDPPEventAggregation(TheSettings->EventsBeforeReadout, 0);
    
    DGMgr->SetRunSynchronizationMode(CAEN_DGTZ_RUN_SYNC_Disabled);

    
    ////////////////////////////////////
    // Set PSD analog and virtual probes
    
    DGMgr->SetDPPVirtualProbe(ANALOG_TRACE_1,
			      CAEN_DGTZ_DPP_VIRTUALPROBE_Input);
    
    DGMgr->SetDPPVirtualProbe(ANALOG_TRACE_2,
			      CAEN_DGTZ_DPP_VIRTUALPROBE_Baseline);

    // DPP-PSD digital traces have the following configuration:
    //  - Digital trace 1 is fixed to the channel self-trigger threshold
    //  - Digital trace 2 is fixed to the PSD integration long gate
    //  - Digital trace 3 has the following options:
    //     CAEN_DGTZ_DPP_DIGITALPROBE_ExtTRG;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_OverThr;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_TRGOut;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_CoincWin;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_PileUp;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_Coincidence;
    // - Digital trace 4 has the following options:
    //     CAEN_DGTZ_DPP_DIGITALPROBE_GateShort;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_OverThr;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_TRGVal;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_TRGHoldoff;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_PileUp;
    //     CAEN_DGTZ_DPP_DIGITALPROBE_Coincidence;

    Int_t Status = -42;
    
    Status = DGMgr->SetDPPVirtualProbe(DIGITAL_TRACE_3,
				       CAEN_DGTZ_DPP_DIGITALPROBE_PileUp);
    
    Status = DGMgr->SetDPPVirtualProbe(DIGITAL_TRACE_4,
				       CAEN_DGTZ_DPP_DIGITALPROBE_GateShort);
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
