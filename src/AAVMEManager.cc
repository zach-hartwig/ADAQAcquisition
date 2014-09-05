#include "ADAQBridge.hh"
#include "ADAQDigitizer.hh"
#include "ADAQHighVoltage.hh"

#include "AAVMEManager.hh"


AAVMEManager *AAVMEManager::TheVMEManager = 0;


AAVMEManager *AAVMEManager::GetInstance()
{ return TheVMEManager; }


AAVMEManager::AAVMEManager()
  : BREnable(false), DGEnable(true), HVEnable(false),
    DGAddress(0x00000000), HVAddress(0x42420000),
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


//void AAInterface::RunDGScope()
//{
/*

  // Get the data thinning factor 
  bool UseDataReduction = DGScopeUseDataReduction_CB->IsDown();
  uint32_t DataReductionFactor = DGScopeDataReductionFactor_NEL->GetEntry()->GetIntNumber();

  if(RecordLength % DataReductionFactor != 0 && UseDataReduction){
    cout << "\nError! (RecordLength % DataReductionFactor) MUST equal zero to avoid grevious errors!\n"
	 <<   "       Adjust the data reduction factor and restart acquisition ...\n"
	 << endl;

    StopAcquisitionSafely();
  }
  
  // Variables for graphing the digitized waveforms as time versus
  // voltage. Units are determined by the user's selections when the
  // arrays are filled inside the acquisition loop
  double Time_graph[RecordLength], Voltage_graph[RecordLength]; 

   
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

    // Get each channel's baseline calculation region (min, max, length)
    BaselineCalcMin[ch] = DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber(); // [sample]
    BaselineCalcMax[ch] = DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber(); // [sample]
    BaselineCalcLength[ch] = BaselineCalcMax[ch]-BaselineCalcMin[ch]; // [sample]
    BaselineCalcResult[ch] = 0; // [ADC] Result is calculated during acquisition

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

//  bool DebugModeEnabled = false;
//  int DebugModeWaveformGenerationPause = 1000;
//  if(BuildInDebugMode){
//    DebugModeEnabled = DebugModeEnable_CB->IsDown();
//    DebugModeWaveformGenerationPause = DebugModeWaveformGenerationPause_NEL->GetEntry()->GetIntNumber();
//  }

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


    //    long elapsedA_sec = preA.tv_sec - postA.tv_sec;
    //    long elapsedA_nsec = 0;
    //    if(postA.tv_nsec > preA.tv_nsec)
    //      elapsedA_nsec = postA.tv_nsec - preA.tv_nsec;
    //    else
    //      elapsedA_nsec = (1e9 + postA.tv_nsec) - preA.tv_nsec;
    
    //    cout << std::dec 
    //	 << "Elapsed time: " << elapsedA_sec << "s \t " << elapsedA_nsec << "ns" 
    //	 << endl;

  } // End DGScope acquisition 'while' loop
  
  // Once the acquisition session has concluded, free the memory that
  // was allocated to the V1720 and PC event buffers
  DGManager->FreeReadoutBuffer(&Buffer);
  */
//}

/*
void AAInterface::SaveSpectrumData()
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




// Method to safely cease acquiring data, including writing and
// closing of possibly opened ROOT files.
void AAInterface::StopAcquisitionSafely()
{
  // Stop the V1720 from acquiring data first thing
  TheVMEManager->GetDGManager()->SWStopAcquisition();
  
  
  
  // Set bool to disable the DGScope loop
  DGScopeEnable = false;
    
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
*/

// Method to generate a standard detector-esque artificial waveoforms
// to be used in debugging mode when waveforms from the DAQ are not
// available. Method receives the record length of the acquisition
// window and a reference to the being-processed data channel in order
// to fill the channel data with the artificial waveform data. The
// artificial waveform has a quick rise time and longer decay tail,
// which are randomly varied to mimick data acquisition.
/*
void AAInterface::GenerateArtificialWaveform(int RecordLength, vector<int> &Voltage, 
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
*/



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
