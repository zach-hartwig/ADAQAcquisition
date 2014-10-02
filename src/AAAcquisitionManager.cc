#include <TSystem.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <bitset>

#include "AAAcquisitionManager.hh"
#include "AAVMEManager.hh"
#include "AAGraphics.hh"


AAAcquisitionManager *AAAcquisitionManager::TheAcquisitionManager = 0;


AAAcquisitionManager *AAAcquisitionManager::GetInstance()
{ return TheAcquisitionManager; }


AAAcquisitionManager::AAAcquisitionManager()
  : AcquisitionEnable(false),
    AcquisitionTimeStart(0), AcquisitionTimeStop(0), 
    AcquisitionTimeNow(0), AcquisitionTimePrev(0),
    EventPointer(NULL), EventWaveform(NULL), Buffer(NULL),
    BufferSize(0), ReadSize(0), FPGAEvents(0), PCEvents(0),
    WaveformLength(0), LLD(0), ULD(0), SampleHeight(0.), TriggerHeight(0.),
    PulseHeight(0.), PulseArea(0.),
    FillWaveformTree(false)
{
  if(TheAcquisitionManager)
    cout << "\nError! The AcquisitionManager was constructed twice!\n" << endl;
  TheAcquisitionManager = this;
  
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  
  int DGChannels = DGManager->GetNumChannels();
  
  for(int ch=0; ch<DGManager->GetNumChannels(); ch++){

    BufferFull.push_back(true);
    
    BaselineStart.push_back(0);
    BaselineStop.push_back(0);
    BaselineLength.push_back(0);
    BaselineValue.push_back(0);
    
    Polarity.push_back(0.);
    
    CalibrationDataStruct DataStruct;
    CalibrationData.push_back(DataStruct);
    CalibrationEnable.push_back(false);
    CalibrationCurves.push_back(new TGraph);
    
    Spectrum_H.push_back(new TH1F);
    SpectrumExists.push_back(true);
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
  
  WaveformLength = TheSettings->RecordLength;
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
    
    if(SpectrumExists[ch]) {
      delete Spectrum_H[ch];
      SpectrumExists[ch] = false;
    }
    else 
      continue;
    
    stringstream SS;
    SS << "Spectrum[Ch" << ch << "]";
    string Name = SS.str();
    
    Spectrum_H[ch] = new TH1F(Name.c_str(), 
			      Name.c_str(), 
			      TheSettings->SpectrumNumBins,
			      TheSettings->SpectrumMinBin,
			      TheSettings->SpectrumMaxBin);

    SpectrumExists[ch] = true;
  }
  
  // GraphicsManager settings

  if(TheSettings->WaveformMode)
    AAGraphics::GetInstance()->SetupWaveformGraphics(WaveformLength);
  else if(TheSettings->SpectrumMode)
    AAGraphics::GetInstance()->SetupSpectrumGraphics();
	
  // Initialize CAEN digitizer readout data for acquisition

  EventPointer = NULL;
  EventWaveform = NULL;
  Buffer = NULL;
  BufferSize = ReadSize = FPGAEvents = PCEvents = 0;
  
  AAVMEManager::GetInstance()->GetDGManager()->MallocReadoutBuffer(&Buffer, &BufferSize);
}


void AAAcquisitionManager::StartAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();

  AAGraphics *TheGraphicsManager = AAGraphics::GetInstance();

  // Check to ensure at least one channel is enabled

  uint32_t ChannelEnableMask = 0;
  DGManager->GetChannelEnableMask(&ChannelEnableMask);

  if((0xff & ChannelEnableMask)==0){
    cout << "\tError! At least one channel must be enabled! Stopping acquisition ...\n"
	 << endl;
    return;
  }

  // Initialize all variables before acquisition begins

  PreAcquisition();

  // Start the acquisition and enable the acquisition bool

  DGManager->SWStartAcquisition();

  AcquisitionEnable = true;

  while(AcquisitionEnable){
    
    // Handle acquisition actions in separate ROOT thread
    gSystem->ProcessEvents();

    // Get the number of events stored in the FPGA buffer and maximize
    // efficiency by entering the event readout loop only when the max
    // number of readout events has been reached
    
    DGManager->GetNumFPGAEvents(&FPGAEvents);
    if(FPGAEvents < TheSettings->EventsBeforeReadout)
      continue;
    
    // Perform the FPGA buffer -> PC buffer data readout
    DGManager->ReadData(Buffer, &ReadSize);    
    
    // Get the number of events in the PC buffer
    DGManager->GetNumEvents(Buffer, ReadSize, &PCEvents);
    
    // For each event stored in the PC memory buffer...
    for(uint32_t evt=0; evt<PCEvents; evt++){
      
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

	BaselineValue[ch] = PulseHeight = PulseArea = 0.;
	
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
	} // End sample loop

	if(!TheSettings->UltraRateMode){
	  
	  // Use the calibration curves (ROOT TGraph's) to convert
	  // the pulse height/area from ADC to the user's energy
	  // units using linear interpolation calibration points
	  if(CalibrationEnable[ch]){

	    if(TheSettings->SpectrumPulseHeight)
	      PulseHeight = CalibrationCurves[ch]->Eval(PulseHeight);
	    else
	      PulseArea = CalibrationCurves[ch]->Eval(PulseArea);
	  }
	  
	  // Pulse height spectrum
	  if(TheSettings->SpectrumPulseHeight){

	    if(TheSettings->LDEnable){
	      if(PulseHeight > LLD and PulseHeight < ULD)
		Spectrum_H[ch]->Fill(PulseHeight);
	    }
	    else
	      Spectrum_H[ch]->Fill(PulseHeight);
	    
	    if(TheSettings->LDTrigger and ch == TheSettings->LDChannel)
	      FillWaveformTree = true;
	  }
	  
	  // Pulse area spectrum
	  else{
	    if(TheSettings->LDEnable){
	      if(PulseArea > LLD and PulseArea < ULD)
		Spectrum_H[ch]->Fill(PulseArea);
	    }
	    else
	      Spectrum_H[ch]->Fill(PulseArea);
	    
	    if(TheSettings->LDTrigger and ch == TheSettings->LDChannel)
	      FillWaveformTree = true;
	  }
	}
      } // End digitizer channel loop
      
      if(TheSettings->WaveformStorageEnable){
	
	// If the user has specified that the LLD/ULD should be used
	// as the "trigger" (for plotting the PAS/PHS and writing to a
	// ROOT file) but the present waveform is NOT within the
	// LLD/ULD window (indicated by the DiscrOKForOutput bool set
	// above during analysis of the readout waveform then do NOT
	// write the waveform to the ROOT TTree

	if(TheSettings->LDEnable and !FillWaveformTree)
	  continue;
	
	WaveformTree->Fill();
	
	// Reset the bool used to determine if the LLD/ULD window
	// should be used as the "trigger" for writing waveforms
	FillWaveformTree = false;
      }
      
      // Plot the waveform; only plot the first waveform event in the
      // case of a multievent readout to maximize efficiency

      if(TheSettings->WaveformMode and evt == 0)
	TheGraphicsManager->PlotWaveforms(Waveforms, 
					  WaveformLength, 
					  BaselineValue);
    } // End readout event loop
    
    if(TheSettings->SpectrumMode){
      
      // Use user set value for spectrum refresh rate to determine
      // when to update the spectrum histogram

      int Entries = Spectrum_H[TheSettings->SpectrumChannel]->GetEntries();
      int Rate = TheSettings->SpectrumRefreshRate;
      if(Entries % Rate == 0)
	TheGraphicsManager->PlotSpectrum(Spectrum_H[TheSettings->SpectrumChannel]);
    }
    
    // Free the PC memory allocated to the event
    DGManager->FreeEvent(&EventWaveform);

  } // End acquisition loop
  
  // Free the PC memory allocated to the readout buffer
  DGManager->FreeReadoutBuffer(&Buffer);
}


void AAAcquisitionManager::StopAcquisition()
{
  AAVMEManager::GetInstance()->GetDGManager()->SWStopAcquisition();

  AcquisitionEnable = false;
  
  if(AcquisitionTimerEnable){
    AcquisitionTimerEnable = false;
    TheInterface->UpdateAfterAQTimerStopped(ADAQFileIsOpen);
  }
  
  if(ADAQFileIsOpen)
    CloseADAQFile();
}


void AAAcquisitionManager::SaveSpectrum(string FileName)
{
  int Channel = TheSettings->SpectrumChannel;

  if(!SpectrumExists[Channel])
    return;

  size_t Found = FileName.find_last_of(".");
  if(Found != string::npos){

    string FileExtension = FileName.substr(Found, string::npos);
    
    if(FileExtension == ".dat" or FileExtension == ".csv"){
      
      if(TheSettings->SpectrumSaveWithTimeExtension){
	time_t Time = time(NULL);
	stringstream SS;
	SS << "." << Time;
	string TimeString = SS.str();
	
	FileName.insert(Found, TimeString);
      }
      
      // Create an ofstream object to write the data to a file
      ofstream SpectrumOutput(FileName.c_str(), ofstream::trunc);
      
      // Assign the data separator based on file extension
      string Separator;
      if(FileExtension == ".dat")
	Separator = "\t";
      else if(FileExtension == ".csv")
	Separator = ",";
      
      // Get the number of bins in the spectrum histogram
      int NumBins = Spectrum_H[Channel]->GetNbinsX();
      
      // Iterate over all the bins in spectrum histogram and output
      // the bin center (value on the X axis of the histogram) and the
      // bin content (value on the Y axis of the histogram)
      for(int bin=1; bin<=NumBins; bin++){
	double BinCenter = Spectrum_H[Channel]->GetBinCenter(bin);
	double BinContent = Spectrum_H[Channel]->GetBinContent(bin);
	
	SpectrumOutput << BinCenter << Separator << BinContent
		       << endl;
      }
      
      // Close the ofstream object
      SpectrumOutput.close();
    }
    else if(FileExtension == ".root"){
      TFile *SpectrumOutput = new TFile(FileName.c_str(), "recreate");
      Spectrum_H[Channel]->Write("Spectrum");
      SpectrumOutput->Close();
    }
    else{
    }
  }
}


bool AAAcquisitionManager::AddCalibrationPoint(int Channel, int SetPoint,
					       double Energy, double PulseUnit)
{
  if(SetPoint == CalibrationData[Channel].PointID.size()){
    CalibrationData[Channel].PointID.push_back(SetPoint);
    CalibrationData[Channel].Energy.push_back(Energy);
    CalibrationData[Channel].PulseUnit.push_back(PulseUnit);
  }
  else{
    CalibrationData[Channel].Energy[SetPoint] = Energy;
    CalibrationData[Channel].PulseUnit[SetPoint] = PulseUnit;
  }
  
  return true;
}


bool AAAcquisitionManager::EnableCalibration(int Channel)
{
  // If there are 2 or more points in the current channel's
  // calibration data set then create a new TGraph object. The
  // TGraph object will have pulse units [ADC] on the X-axis and the
  // corresponding energies [in whatever units the user has entered
  // the energy] on the Y-axis. A TGraph is used because it provides
  // very easy but extremely powerful methods for interpolation,
  // which allows the pulse height/area to be converted in to energy
  // efficiently in the acquisition loop.
  
  if(CalibrationData[Channel].PointID.size() >= 2){
    
    // Determine the total number of calibration points in the
    // current channel's calibration data set
    const int NumPoints = CalibrationData[Channel].PointID.size();
    
    // Create a new "CalibrationManager" TGraph object.
    CalibrationCurves[Channel] = new TGraph(NumPoints,
					    &CalibrationData[Channel].PulseUnit[0],
					    &CalibrationData[Channel].Energy[0]);
    
    // Set the current channel's calibration boolean to true,
    // indicating that the current channel will convert pulse units
    // to energy within the acquisition loop before histogramming
    // the result into the channel's spectrum
    CalibrationEnable[Channel] = true;

    return true;
  }
  else
    return false;
}


bool AAAcquisitionManager::ResetCalibration(int Channel)
{
  // Clear the channel calibration vectors for the current channel
  CalibrationData[Channel].PointID.clear();
  CalibrationData[Channel].Energy.clear();
  CalibrationData[Channel].PulseUnit.clear();
  
  // Delete the current channel's depracated calibration manager
  // TGraph object to prevent memory leaks
  if(CalibrationEnable[Channel])
    delete CalibrationCurves[Channel];
  
  // Set the current channel's calibration boolean to false,
  // indicating that the calibration manager will NOT be used within
  // the acquisition loop
  CalibrationEnable[Channel] = false;
  
  return true;
}


bool AAAcquisitionManager::LoadCalibration(int Channel, string FileName, int &NumPoints)
{
  size_t Found = FileName.find_last_of(".");
  if(Found != string::npos){
    
    // Set the calibration file to an input stream
    ifstream In(FileName.c_str());
    
    // An index to control the set point
    int SetPoint = 0;
    
    // Iterate through each line in the file and use the values
    // (column1 == energy, column2 == pulse unit) to set the
    // CalibrationData objects for the current channel
    while(In.good()){
      double Energy, PulseUnit;
      In >> Energy >> PulseUnit;
      
      if(In.eof()) break;
      
      AddCalibrationPoint(Channel, SetPoint, Energy, PulseUnit);
      
      SetPoint++;
    }

    NumPoints = SetPoint;

    return true;
  }
  else
    return false;
}


bool AAAcquisitionManager::WriteCalibration(int Channel, string FileName)
{
  // Set the calibration file to an input stream
  ofstream Out(FileName.c_str());
  
  for(int i=0; i<CalibrationData[Channel].Energy.size(); i++)
    Out << setw(10) << CalibrationData[Channel].Energy[i]
	<< setw(10) << CalibrationData[Channel].PulseUnit[i]
	<< endl;
  Out.close();

  return true;
}


void AAAcquisitionManager::CreateADAQFile(string FileName)
{
  if(ADAQFileIsOpen)
    return;
  
  ADAQFile = new TFile(FileName.c_str(), "recreate");
  
  Comment = new TObjString("Comments are not presently enabled! ZSH 14 Apr 14");

  Parameters = new ADAQRootMeasParams();

  WaveformTree = new TTree("WaveformTree","Prototype tree to store all waveforms of an event");

  int DGChannels = AAVMEManager::GetInstance()->GetDGManager()->GetNumChannels();
  for(int ch=0; ch<DGChannels; ch++){
    ostringstream SS;

    // Create a channel-specific branch name...
    SS << "VoltageInADC_Ch" << ch;
    string BranchName = SS.str();
    
    // ...and use it to specify a channel-specific branch in
    /// the waveform TTree. The branch holds the address of the
    // vector that contains the digitized waveform
    WaveformTree->Branch(BranchName.c_str(), &Waveforms[ch]);
  }


  // Retrieve the present voltage and drawn current for each
  // high voltage channel and store in the MeasParam object
  uint16_t voltage = 0;
  uint16_t current = 0;

  for(int ch=0; ch<6; ch++){
    Parameters->DetectorVoltage.push_back(0);
    Parameters->DetectorCurrent.push_back(0);
  }


  // Retrieve the present settings for each of the digitizer
  // channels and store in the MeasParam object

  for(int ch=0; ch<8; ch++){
    Parameters->DCOffset.push_back(TheSettings->ChDCOffset[ch]);
    Parameters->TriggerThreshold.push_back(TheSettings->ChTriggerThreshold[ch]);
    Parameters->BaselineCalcMin.push_back(TheSettings->ChBaselineCalcMin[ch]);
    Parameters->BaselineCalcMax.push_back(TheSettings->ChBaselineCalcMax[ch]);
  }
      
  // Retrieve the record length for the acquisition window [samples].
  Parameters->RecordLength = TheSettings->RecordLength;

  // If the user has selected to reduce the output data then modify
  // the record length accordingly. Note that this effectively
  // destroys any pulse timing information, but it presently done to
  // avoid modifying the structure of the ADAQ ROOT files. In the
  // future, this should be correctly implemented. ZSH 26 AUG 13
  if(TheSettings->DataReductionEnable)
    Parameters->RecordLength /= TheSettings->DataReductionFactor;
  
  ADAQFileIsOpen = true;
}


void AAAcquisitionManager::CloseADAQFile()
{
  if(!ADAQFileIsOpen)
    return;
  
  if(WaveformTree)
    WaveformTree->Write();
  
  Parameters->Write("MeasParams");
  //Comment->Write("MeasComment");
  
  ADAQFile->Close();
  
  // Free the memory allocated to ROOT objects
  delete Parameters;
  //delete WaveformTree;
  delete Comment;
  delete ADAQFile;
    
  ADAQFileIsOpen = false;
}


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



/*
  int LowestEnabledChannel = 0;
  uint32_t LowestChannelMask = 0x1;
  while(!(ChannelEnableMask & LowestChannelMask)){
  LowestChannelMask <<= 1;
  LowestEnabledChannel++;
  }
*/
