#include <TSystem.h>

#include <iostream>
#include <sstream>

#include "AAAcquisitionManager.hh"
#include "AAVMEManager.hh"


AAAcquisitionManager *AAAcquisitionManager::TheAcquisitionManager = 0;


AAAcquisitionManager *AAAcquisitionManager::GetInstance()
{ return TheAcquisitionManager; }


AAAcquisitionManager::AAAcquisitionManager()
  : AcquisitionEnable(false),
    AcquisitionTimeStart(0), AcquisitionTimeStop(0), 
    AcquisitionTimeNow(0), AcquisitionTimePrev(0),
    EventPointer(NULL), EventWaveform(NULL), Buffer(NULL),
    BufferSize(0), ReadSize(0), NumEvents(0),
    LLD(0), ULD(0), SampleHeight(0.), TriggerHeight(0.),
    PulseHeight(0.), PulseArea(0.),
    BranchWaveformTree(false),
    WriteWaveformToTree(false)
{
  if(TheAcquisitionManager)
    cout << "\nError! The AcquisitionManager was constructed twice!\n" << endl;
  TheAcquisitionManager = this;
  
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  
  int DGChannels = DGManager->GetNumChannels();
  
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
    BaselineStart.push_back(0);
    BaselineStop.push_back(0);
    BaselineLength.push_back(0);
    BaselineValue.push_back(0);
    
    Polarity.push_back(0.);
    
    CalibrationEnable.push_back(false);
    CalibrationCurves.push_back(new TGraph);
    
    Spectrum_H.push_back(new TH1F);
  }
}


AAAcquisitionManager::~AAAcquisitionManager()
{;}


void AAAcquisitionManager::PreAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();

  int DGChannels = DGManager->GetNumChannels();

  /////////////////////////////////////////////////
  // Initialize general member data for acquisition

  // Acquisition timer

  AcquisitionTimeNow = 0;
  AcquisitionTimePrev = 0;

  // Baseline calculation
  
  for(int ch=0; ch<DGChannels; ch++){
    BaselineStart[ch] = TheSettings->ChBaselineCalcMin[ch];
    BaselineStop[ch] = TheSettings->ChBaselineCalcMax[ch];
    BaselineLength[ch] = BaselineStop[ch] - BaselineStart[ch];
    BaselineValue[ch] = 0.;
    
    if(TheSettings->ChPosPolarity[ch])
      Polarity[ch] = 1.;
    else
      Polarity[ch] = -1.;
  }
  
  // Waveform readout
  
  int WaveformLength = TheSettings->RecordLength;
  if(TheSettings->DataReductionEnable)
    WaveformLength /= TheSettings->DataReductionFactor;
  
  Waveforms.clear();
  Waveforms.resize(DGChannels);
  for(int ch=0; ch<DGChannels; ch++){
    if(TheSettings->ChEnable[ch])
      Waveforms[ch].resize(WaveformLength);
    else
      Waveforms[ch].resize(0);
  }

  // Calibration

  // Pulse spectra creation
  LLD = TheSettings->SpectrumLLD;
  ULD = TheSettings->SpectrumULD;

  for(int ch=0; ch<DGChannels; ch++){
    stringstream SS;
    SS << "Spectrum[Ch" << ch << "]";
    string Name = SS.str();

    if(Spectrum_H[ch]) delete Spectrum_H[ch];
    
    Spectrum_H[ch] = new TH1F(Name.c_str(), 
			      Name.c_str(), 
			      TheSettings->SpectrumNumBins,
			      TheSettings->SpectrumMinBin,
			      TheSettings->SpectrumMaxBin);
  }
  
  // Initialize CAEN digitizer readout data for acquisition

  EventPointer = NULL;
  EventWaveform = NULL;
  Buffer = NULL;
  BufferSize = ReadSize = NumEvents = 0;
  
  AAVMEManager::GetInstance()->GetDGManager()->MallocReadoutBuffer(&Buffer, &BufferSize);
}


void AAAcquisitionManager::StartAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  
  // Initialize all variables before acquisition begins
  PreAcquisition();

  DGManager->SWStartAcquisition();

  AcquisitionEnable = true;

  while(AcquisitionEnable){
    
    // Handle acquisition actions in separate ROOT thread
    gSystem->ProcessEvents();
    
    // Get the number of events stored in the FPGA buffer
    DGManager->GetRegisterValue(0x812c, &NumEvents);
    
    // Prevent FPGA->PC readout unless the number events exceeds the
    // number specified by the user explicity for readout (efficiency)
    if(NumEvents < TheSettings->EventsBeforeReadout)
      continue;

    // Perform the FPGA buffer -> PC buffer data readout
    DGManager->ReadData(Buffer, &ReadSize);    
    
    // For each event stored in the PC memory buffer...
    for(uint32_t evt=0; evt<NumEvents; evt++){

      if(AcquisitionTimerEnable){
	
	// Calculate the elapsed time since the timer was started
	AcquisitionTimePrev = AcquisitionTimeNow;
	AcquisitionTimeNow = time(NULL) - AcquisitionTimeStart; // [seconds]
	
	// Update the AQTimer widget only every second
	if(AcquisitionTimePrev != AcquisitionTimeNow){
	  int TimeRemaining = AcquisitionTimeStop - AcquisitionTimeNow;
	  TheInterface->UpdateAQTimer(TimeRemaining);
	}
	
	// If the timer has reached zero, i.e. the acquisition loop has
	// run for the duration specified, then turn the acquisition off
	if(AcquisitionTimeNow >= AcquisitionTimeStop)
	  StopAcquisition();
      }
      
      // Populate the EventInfo structure and assign address to the EventPointer
      DGManager->GetEventInfo(Buffer, ReadSize, evt, &EventInfo, &EventPointer);
      
      //  Fill the EventWaveform structure with digitized signal voltages
      DGManager->DecodeEvent(EventPointer, &EventWaveform);

      // Prevent waveform analysis if there is nothing to analyze,
      // i.e. seg-fault protection just in case!
      if(EventWaveform == NULL)
	continue;
      
      // Iterate over the digitizer channels that are enabled
      for(int ch=0; ch<DGManager->GetNumChannels(); ch++){
	
	if(!TheSettings->ChEnable[ch])
	  continue;

	PulseHeight = PulseArea = 0.;
	
	for(int sample=0; sample<TheSettings->RecordLength; sample++){

	  /////////////////////////
	  // Data reduction readout

	  if(TheSettings->DataReductionEnable){
	    if(sample % TheSettings->DataReductionFactor == 0){
	      int Index = sample / TheSettings->DataReductionFactor;
	      Waveforms[ch][Index] = EventWaveform->DataChannel[ch][sample];
	    }
	  }


	  ///////////////////
	  // Standard readout

	  else
	    Waveforms[ch][sample] = EventWaveform->DataChannel[ch][sample];


	  ///////////////////
	  // Pulse processing

	  if(!TheSettings->UltraRateMode){
	  
	    // Calculate the baseline by taking the average of all
	    // samples that fall within the baseline calculation region
	    if(sample > BaselineStart[ch] and sample <= BaselineStop[ch])
	      BaselineValue[ch] += Waveforms[ch][sample] * 1.0 / BaselineLength[ch]; // [ADC]
	    
	    // Analyze the pulses to obtain pulse spectra
	    else if(sample >= BaselineStop[ch]){
	      
	      // Calculate the waveform sample distance from the baseline
	      SampleHeight = Polarity[ch] * (Waveforms[ch][sample] - BaselineValue[ch]);
	      TriggerHeight = Polarity[ch] * (TheSettings->ChTriggerThreshold[ch] - BaselineValue[ch]);
	      
	      // Simple algorithm to determine maximum peak height in the pulse
	      if(SampleHeight > PulseHeight)// and SampleHeight > TriggerHeight)
		PulseHeight = SampleHeight;
	      
	      // Integrate the area under the pulse
	      if(SampleHeight > TriggerHeight)
		PulseArea += SampleHeight;
	    }
	  }
	}

	if(!TheSettings->UltraRateMode){
	  
	  if(CalibrationEnable[ch]){
	    
	    // Use the calibration curves (ROOT TGraph's) to convert
	    // the pulse height/area from ADC to the user's energy
	    // units using linear interpolation calibration points
	    if(TheSettings->SpectrumPulseHeight)
	      PulseHeight = CalibrationCurves[ch]->Eval(PulseHeight);
	    else
	      PulseArea = CalibrationCurves[ch]->Eval(PulseArea);
	  }
	  

	  // Pulse height spectrum
	  if(TheSettings->SpectrumPulseHeight){
	    if(PulseHeight > LLD and PulseHeight < ULD){
	      Spectrum_H[ch]->Fill(PulseHeight);
	      
	      if(TheSettings->LDEnable and ch == TheSettings->LDChannel)
		WriteWaveformToTree = true;
	    }
	  }
	  
	  // Pulse area spectrum
	  else{
	    if(PulseArea > LLD and PulseArea < ULD){
	      Spectrum_H[ch]->Fill(PulseArea);
	      
	      if(TheSettings->LDEnable and ch == TheSettings->LDChannel)
		WriteWaveformToTree = true;
	    }
	  }
	}
	
	if(BranchWaveformTree)
	  CreateWaveformTreeBranches();
	
      }

      if(WaveformTree and false){//TheSettings->DataStorageEnable){
	
	// If the user has specified that the LLD/ULD should be used
	// as the "trigger" (for plotting the PAS/PHS and writing to a
	// ROOT file) but the present waveform is NOT within the
	// LLD/ULD window (indicated by the DiscrOKForOutput bool set
	// above during analysis of the readout waveform then do NOT
	// write the waveform to the ROOT TTree
	if(TheSettings->LDEnable and !WriteWaveformToTree)
	  continue;
	
	WaveformTree->Fill();
	
	// Reset the bool used to determine if the LLD/ULD window
	// should be used as the "trigger" for writing waveforms
	WriteWaveformToTree = false;
      }





    }
    // Free the PC memory allocated to the event
    DGManager->FreeEvent(&EventWaveform);
  }

  // Free the PC memory allocated to the readout buffer
  DGManager->FreeReadoutBuffer(&Buffer);
}


void AAAcquisitionManager::StopAcquisition()
{
  AAVMEManager::GetInstance()->GetDGManager()->SWStopAcquisition();

  AcquisitionEnable = false;

  if(AcquisitionTimerEnable){
    AcquisitionTimerEnable = false;
    TheInterface->UpdateAfterAQTimerStopped(ROOTFileOpen);
  }

  if(ROOTFileOpen){
  }
}



void AAAcquisitionManager::CreateWaveformTreeBranches()
{
  int DGChannels = AAVMEManager::GetInstance()->GetDGManager()->GetNumChannels();
  
  for(int ch=0; ch<DGChannels; ch++){
    ostringstream SS;

    // Create a channel-specific branch name...
    SS << "VoltageInADC_Ch" << ch;
    string BranchName = SS.str();
    
    // ...and use it to specify a channel-specific branch in
    /// the waveform TTree The branch holds the address of the
    // vector that contains the waveform as a function of
    // record length and the RecordLength of each waveform
    WaveformTree->Branch(BranchName.c_str(), &Waveforms[ch]);
  }

  // Set the boolean to false to prevent recreating branches during
  // next iteration of the acquisition loop
  BranchWaveformTree = false;
}

  



/*
  // Stop the V1720 from acquiring data first thing
  TheVMEManager->GetDGManager()->SWStopAcquisition();
  
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
  }
*/


/*

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
   
   
  // The following variables are declared here but used/set within
  // the acquisition loop

  // Variables for plotted waveform vertical offset and plotted
  // channel trigger threshold. Note that these variables are set
  // dynamicallly within the acquisition loop to allow the user
  // flexibility in setting these values
  double VertPosOffset, ChTrigThr;
  VertPosOffset = ChTrigThr = 0;

  // The active channel to histogram
  uint32_t ChannelToHistogram = 0;

  // Set the style of the histogram statistics box
  gStyle->SetOptStat("ne");
   
  // Convert the time to the user's desired units for graphing
 // the waveform [sample or ns]
  for(uint32_t sample=0; sample<RecordLength; sample++)
    Time_graph[sample] = sample * ConvertTimeToGraphUnits;

  // Assign values to use if built in debug mode (Not used)


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

*/





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

