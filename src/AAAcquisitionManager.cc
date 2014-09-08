#include <TSystem.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

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
    BufferSize(0), ReadSize(0), NumEvents(0),
    WaveformLength(0), LLD(0), ULD(0), SampleHeight(0.), TriggerHeight(0.),
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
    
    CalibrationDataStruct DataStruct;
    CalibrationData.push_back(DataStruct);
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
  
  // GraphicsManager settings
  if(TheSettings->WaveformMode)
    AAGraphics::GetInstance()->SetupWaveformGraphics(WaveformLength);
  
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
	}

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

    if(TheSettings->WaveformMode)
      TheGraphicsManager->PlotWaveforms(Waveforms, 
					WaveformLength, 
					BaselineValue);
    else if(TheSettings->SpectrumMode)
      TheGraphicsManager->PlotSpectrum(Spectrum_H);

    
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
    //if(TheInterface->WaveformEnable_CB->IsDown())
    //      {}
    //TheInterface->WaveformEnable_CB->Clicked();

    //TheInterface->WaveformCloseFile_TB->Clicked();
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

  
   
  // Determine the lowest (ie, closest of 0) channel that is enabled
  // in the ChannelEnableMask. 'Fraid you'll have to figure out the
  // bitwise operations on your own if you don't know them...but take
  // it from me, this is a pretty pro'n'shit way to do it
/*
  int LowestEnabledChannel = 0;
  uint32_t LowestChannelMask = 0x1;
  while(!(ChannelEnableMask & LowestChannelMask)){
    LowestChannelMask <<= 1;
    LowestEnabledChannel++;
  }
*/


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
      

void AAAcquisitionManager::SaveSpectrumData()
{
  /*
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
  */
}



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




void AAAcquisitionManager::CreateWaveformFile()
{
  /*
  string FileName = DataFileName + DataFileExtension;

  // TFile to create a ROOT binary file for output                                                                                                                                                                                                                           
  OutputDataFile = new TFile(FileName.c_str(), "recreate");

  // TTree to store the waveforms as arrays. The array indices are
  // sample numbers and array values are the voltages
  WaveformTree = new TTree("WaveformTree","Prototype tree to store all waveforms of an event");

  // TObjString to hold a comment on the measurement data                                                                                                                                                                                                                    
  MeasComment = new TObjString("Comments are not presently enabled! ZSH 14 Apr 14");

  // ADAQ class to hold measurement paremeters                                                                                                                                                                                                                               
  MeasParams = new ADAQRootMeasParams();

  // Retrieve the present voltage and drawn current for each                                                                                                                                                                                                                 
  // high voltage channel and store in the MeasParam object                                                                                                                                                                                                                  
  uint16_t voltage = 0;
  uint16_t current = 0;

  for(int ch=0; ch<HVManager->GetNumChannels(); ch++){                                                                                                                                                                                                                       
    if(V6534Enable){                                                                                                                                                                                                                                                         
      MeasParams->DetectorVoltage.push_back( HVManager->GetVoltage(ch,&voltage) );                                                                                                                                                                                           
      MeasParams->DetectorCurrent.push_back( HVManager->GetCurrent(ch,&current) );                                                                                                                                                                                           
    }                                                                                                                                                                                                                                                                        
    else{                                                                                                                                                                                                                                                                    
      MeasParams->DetectorVoltage.push_back(0);                                                                                                                                                                                                                              
      MeasParams->DetectorCurrent.push_back(0);                                                                                                                                                                                                                              
    }
  }

  // Retrieve the present settings for each of the digitizer                                                                                                                                                                                                                 
  // channels and store in the MeasParam object                                                                                                                                                                                                                              
  for(int ch=0; ch<NumDataChannels; ch++){                                                                                                                                                                                                                                   
    MeasParams->DCOffset.push_back( (int)DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());                                                                                                                                                                               
    MeasParams->TriggerThreshold.push_back( (int)DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() );                                                                                                                                                            
    MeasParams->BaselineCalcMin.push_back( (int)DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber() );                                                                                                                                                                
    MeasParams->BaselineCalcMax.push_back( (int)DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber() );                                                                                                                                                                
  }

  // Retrieve the record length for the acquisition window [samples].                                                                                                                                                                                                        
  MeasParams->RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();                                                                                                                                                                                            
  
  // If the user has selected to reduce the output data then modify                                                                                                                                                                                                          
  // the record length accordingly. Note that this effectively                                                                                                                                                                                                               
  // destroys any pulse timing information, but it presently done to                                                                                                                                                                                                         
  // avoid modifying the structure of the ADAQ ROOT files. In the                                                                                                                                                                                                            
  // future, this should be correctly implemented. ZSH 26 AUG 13                                                                                                                                                                                                             
  if(DGScopeUseDataReduction_CB->IsDown())                                                                                                                                                                                                                                   
    MeasParams->RecordLength /= DGScopeDataReductionFactor_NEL->GetEntry()->GetIntNumber();

  // Set a bool indicating that the next digitized event will                                                                                                                                                                                                                
  // trigger the creation of a TTree branch with the correctly sized                                                                                                                                                                                                         
  // array. This action is performed once in                                                                                                                                                                                                                                 
  // ADAQAcquisition::RunDGScope(). See that function for more comments                                                                                                                                                                                                      
  BranchWaveformTree = true; 

  ROOTFileOpen = true;
  */
}


void AAAcquisitionManager::CloseWaveformFile()
{
  /*
  if(!ROOTFileOpen)
    return;
  
  if(WaveformTree)
      WaveformTree->Write();
  
  // Write the ROOT objects to file
  MeasParams->Write("MeasParams");
  MeasComment->Write("MeasComment");
  
  // Close the ROOT TFile
  OutputDataFile->Close();
  
  // Free the memory allocated to ROOT objects
  delete MeasComment;
  //EventTree->Delete(); // Causes crash for some reason
  delete MeasParams;
  delete OutputDataFile
    
  ROOTFileOpen = false;
  */
}



    /*
    uint16_t voltage = 0;
    uint16_t current = 0;
    
    for(int ch=0; ch<HVManager->GetNumChannels(); ch++){
      if(V6534Enable){
	MeasParams->DetectorVoltage.push_back( HVManager->GetVoltage(ch,&voltage) );
	MeasParams->DetectorCurrent.push_back( HVManager->GetCurrent(ch,&current) );
      }
      else{
	MeasParams->DetectorVoltage.push_back(0);
	MeasParams->DetectorCurrent.push_back(0);
      }
    }
    */
    
    // Retrieve the present settings for each of the digitizer
    // channels and store in the MeasParam object
    //    for(int ch=0; ch<NumDataChannels; ch++){
      //MeasParams->DCOffset.push_back( (int)DGScopeDCOffset_NEL[ch]->GetEntry()->GetHexNumber());
      //      MeasParams->TriggerThreshold.push_back( (int)DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() );
      //      MeasParams->BaselineCalcMin.push_back( (int)DGScopeBaselineCalcMin_NEL[ch]->GetEntry()->GetIntNumber() );
      //      MeasParams->BaselineCalcMax.push_back( (int)DGScopeBaselineCalcMax_NEL[ch]->GetEntry()->GetIntNumber() );
    //    }
    
    // Retrieve the record length for the acquisition window [samples].
    //MeasParams->RecordLength = DGScopeRecordLength_NEL->GetEntry()->GetIntNumber();
    
    // If the user has selected to reduce the output data then modify
    // the record length accordingly. Note that this effectively
    // destroys any pulse timing information, but it presently done to
    // avoid modifying the structure of the ADAQ ROOT files. In the
    // future, this should be correctly implemented. ZSH 26 AUG 13
    //    if(DGScopeUseDataReduction_CB->IsDown())
    //      MeasParams->RecordLength /= DGScopeDataReductionFactor_NEL->GetEntry()->GetIntNumber();
