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
  : AcquisitionEnable(false), AcquisitionTimerEnable(false),
    AcquisitionTimeStart(0), AcquisitionTimeStop(0), 
    AcquisitionTimeNow(0), AcquisitionTimePrev(0),
    EventPointer(NULL), EventWaveform(NULL), Buffer(NULL),
    BufferSize(0), ReadSize(0), FPGAEvents(0), PCEvents(0),
    ReadoutType(0), ReadoutTypeBit(24), ReadoutTypeMask(0b1 << ReadoutTypeBit),
    ZLEEventSizeMask(0x0fffffff), ZLEEventSize(0),
    ZLESampleAMask(0x0000ffff), ZLESampleBMask(0xffff0000), 
    ZLENumWordMask(0x000fffff), ZLEControlMask(0xc0000000),
    WaveformLength(0), LLD(0), ULD(0), SampleHeight(0.), TriggerHeight(0.),
    PulseHeight(0.), PulseArea(0.), PSDTotal(0.), PSDTail(0.),
    PeakPosition(0), TimeStamp(0),
    FillWaveformTree(false), ADAQFileIsOpen(false),
    TheReadoutManager(new ADAQReadoutManager)
{
  if(TheAcquisitionManager)
    cout << "\nError! The AcquisitionManager was constructed twice!\n" << endl;
  TheAcquisitionManager = this;
}


AAAcquisitionManager::~AAAcquisitionManager()
{
  delete TheAcquisitionManager;
  delete TheReadoutManager;
}


void AAAcquisitionManager::Initialize()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  
  Int_t DGChannels = DGManager->GetNumChannels();
  
  for(Int_t ch=0; ch<DGChannels; ch++){
    
    BufferFull.push_back(true);
    
    BaselineStart.push_back(0);
    BaselineStop.push_back(0);
    BaselineLength.push_back(0);
    BaselineValue.push_back(0);

    PSDTotalAbsStart.push_back(0);
    PSDTotalAbsStop.push_back(0);
    PSDTailAbsStart.push_back(0);
    PSDTailAbsStop.push_back(0);
    
    Polarity.push_back(0.);
    
    CalibrationDataStruct DataStruct;
    CalibrationData.push_back(DataStruct);
    CalibrationEnable.push_back(false);
    CalibrationCurves.push_back(new TGraph);
    
    Spectrum_H.push_back(new TH1F);
    SpectrumExists.push_back(true);

    PSDHistogram_H.push_back(new TH2F);
    PSDHistogramExists.push_back(true);
  }
}


void AAAcquisitionManager::PreAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();

  Int_t DGChannels = DGManager->GetNumChannels();

  ////////////////////////////////////////////////////
  // Initialize general member data for acquisition //
  ////////////////////////////////////////////////////

  ////////////////////
  // Acquisition timer

  AcquisitionTimeNow = 0;
  AcquisitionTimePrev = 0;
  

  ///////////////////////
  // Baseline calculation

  for(Int_t ch=0; ch<DGChannels; ch++){
    BaselineStart[ch] = TheSettings->ChBaselineCalcMin[ch];
    BaselineStop[ch] = TheSettings->ChBaselineCalcMax[ch];
    BaselineLength[ch] = BaselineStop[ch] - BaselineStart[ch];
    BaselineValue[ch] = 0.;

    if(TheSettings->ChPosPolarity[ch])
      Polarity[ch] = 1.;
    else
      Polarity[ch] = -1.;
  }
  
  
  ///////////////////
  // Waveform readout

  // Zero length encoding: All digitizer channels (outer vector) are
  // preallocated; the waveform vector (inner vector) memory is _not_
  // preallocated since length of the waveform is unknown due to
  // operation of ZLE algorithm.
  if(TheSettings->ZeroSuppressionEnable){
    Waveforms.clear();
    Waveforms.resize(DGChannels);
  }

  // Standard: All digitizer channels (outer vector) are preallocated;
  // the waveform vector (inner vector) memory _is_ preallocated to
  // store the waveforms since the length (== RecordLength) is
  // constant and known in advance of waveform readout
  else{
    WaveformLength = TheSettings->RecordLength;
    if(TheSettings->DataReductionEnable)
      WaveformLength /= TheSettings->DataReductionFactor;
    
    Waveforms.clear();
    Waveforms.resize(DGChannels);
    for(Int_t ch=0; ch<DGChannels; ch++){
      if(TheSettings->ChEnable[ch])
	Waveforms[ch].resize(WaveformLength);
      else
	Waveforms[ch].resize(0);
    }
  }

  WaveformData.clear();
  for(Int_t ch=0; ch<DGChannels; ch++)
    WaveformData.push_back(new ADAQWaveformData);
  

  /////////////////////////
  // Pulse spectra creation

  LLD = TheSettings->SpectrumLLD;
  ULD = TheSettings->SpectrumULD;

  // Create pulse spectra and PSD histogram objects

  for(Int_t ch=0; ch<DGChannels; ch++){
    
    if(SpectrumExists[ch]){
      delete Spectrum_H[ch];
      SpectrumExists[ch] = false;
      
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

    if(PSDHistogramExists[ch]){
      delete PSDHistogram_H[ch];
      PSDHistogramExists[ch] = false;

      stringstream SS;
      SS << "PSDHistogram[Ch" << ch << "]";
      string Name = SS.str();

      PSDHistogram_H[ch] = new TH2F(Name.c_str(),
				    Name.c_str(),
				    TheSettings->PSDTotalBins,
				    TheSettings->PSDTotalMinBin,
				    TheSettings->PSDTotalMaxBin,
				    TheSettings->PSDTailBins,
				    TheSettings->PSDTailMinBin,
				    TheSettings->PSDTailMaxBin);

      PSDHistogramExists[ch] = true;
    }
  }
    
  // GraphicsManager settings
  
  // In order to maximize readout loop efficiency, any graphical
  // object settings that only need to be set a single time are
  // called once from this pre-acquisition method
  
  if(TheSettings->WaveformMode)
    AAGraphics::GetInstance()->SetupWaveformGraphics(WaveformLength);
  else if(TheSettings->SpectrumMode)
    AAGraphics::GetInstance()->SetupSpectrumGraphics();
  else if(TheSettings->PSDMode)
    AAGraphics::GetInstance()->SetupPSDHistogramGraphics();
  
  // Initialize pointers to the event and event waveform. Memory is
  // preallocated for events here rather than at readout time
  // resulting in slightly larger memory use but faster readout
  EventPointer = NULL;
  EventWaveform = NULL;
  DGManager->AllocateEvent(&EventWaveform);
  
  // Initialize variables for the PC buffer and event readout. Memory
  // is preallocated for the buffer to receive readout events.
  Buffer = NULL;
  BufferSize = ReadSize = FPGAEvents = PCEvents = 0;
  DGManager->MallocReadoutBuffer(&Buffer, &BufferSize);
}


void AAAcquisitionManager::StartAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  
  AAGraphics *TheGraphicsManager = AAGraphics::GetInstance();
  
  // Initialize all variables before acquisition begins
  PreAcquisition();

  // Get the acquisition control setting
  Int_t AcqControl = TheSettings->AcquisitionControl;

  // If acquisition is 'standard' or 'manual' then send the software
  // (SW) signal to begin data acquisition
  if(AcqControl == 0)
    DGManager->SWStartAcquisition();
  
  // If acquisition is 'Gated (NIM/TTL)' then arm the digitizer for
  // reception of S IN signal as data acquisition start/stop 
  else if(AcqControl == 1 or AcqControl == 2)
    DGManager->SInArmAcquisition();
  
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
	  Int_t TimeRemaining = AcquisitionTimeStop - AcquisitionTimeNow;
	  TheInterface->UpdateAQTimer(TimeRemaining);
	}
	
	// If the timer has reached zero, i.e. the acquisition loop has
	// run for the duration specified, then turn the acquisition off
	if(AcquisitionTimeNow >= AcquisitionTimeStop){
	  StopAcquisition();
	  break;
	}
      }
      
      ///////////////////////////////
      // Standard waveform readout //
      ///////////////////////////////

      // Standard waveforms are obtained by using the CAENDigitizer
      // methods encapsulated by the ADAQ libraries

      if(!TheSettings->ZeroSuppressionEnable){

	// Populate the EventInfo structure and assign address to the EventPointer
	DGManager->GetEventInfo(Buffer, ReadSize, evt, &EventInfo, &EventPointer);
	
	// Seg. fault protection
	if(EventPointer == NULL)
	  continue;
	
	//  Fill the EventWaveform structure with digitized signal voltages
	DGManager->DecodeEvent(EventPointer, &EventWaveform);
	
	// Seg. fault protection
	if(EventWaveform == NULL)
	  continue;
      }

      
      ///////////////////////////////////////////
      // Zero length encoding waveform readout //
      ///////////////////////////////////////////

      // ZLE waveforms are obtained using custom ADAQ methods to
      // readout the PC buffer directly into the "Waveforms" data
      // member.
      
      else{
	if(DGManager->GetZLEWaveform(Buffer, evt, Waveforms) != 0){
	  cout << "\nAAAcquisitionManager::StartAcquisition() : You've encountered a serious error!\n"
	       <<   "  There was an error reading out Event[" << evt << "] when using ZLE mode!\n"
	       <<   "  This issue is likely due to using a RecordLength > 4030. This setting causes\n"
	       <<   "  -- CAEN_DGTZ_GetNumEvents() to incorrectly return a '1'\n"
	       <<   "  -- The readout PC buffer is not correctly filled causing algorithm to segfault\n"
	       <<   "  CAEN has been contacted regarding this bug. ZSH (16 Oct 14)\n"
	       << endl;
	  
	  continue;
	}
	//DGManager->PrintZLEEventInfo(Buffer, evt);
      }
      
      // Iterate over the digitizer channels that are enabled
      for(Int_t ch=0; ch<DGManager->GetNumChannels(); ch++){
	
	if(!TheSettings->ChEnable[ch])
	  continue;

	// Reset values used for to compute values at the channel-level
	BaselineValue[ch] = PulseHeight = PulseArea = 0.;
	PSDTotal = PSDTail = 0.;
	
	WaveformData[ch]->Initialize();
	
	Int_t NumSamples = Waveforms[ch].size();
	for(Int_t sample=0; sample<NumSamples; sample++){

	  // For "standard" or "data reduction" readout, the Waveforms
	  // vector-of-vectors must be filled from the CAEN EventWaveform
	  
	  if(!TheSettings->ZeroSuppressionEnable){
	    
	    // Data reduction readout
	    if(TheSettings->DataReductionEnable){
	      if(sample % TheSettings->DataReductionFactor == 0){
		Int_t Index = sample / TheSettings->DataReductionFactor;
		Waveforms[ch][Index] = EventWaveform->DataChannel[ch][sample];
	      }
	    }

	    // Standard readout
	    else
	      Waveforms[ch][sample] = EventWaveform->DataChannel[ch][sample];
	  }
	  
	  //////////////////////////////
	  // On-the-fly pulse processing

	  if(!TheSettings->DisplayNonUpdateable){
	  
	    // Calculate the baseline by taking the average of all
	    // samples that fall within the baseline calculation region
	    if(sample > BaselineStart[ch] and sample <= BaselineStop[ch])
	      BaselineValue[ch] += Waveforms[ch][sample] * 1.0 / BaselineLength[ch]; // [ADC]
	    
	    // Analyze the pulses to obtain pulse spectra
	    else if(sample >= BaselineStop[ch]){
	      
	      // Calculate the waveform sample distance from the baseline
	      SampleHeight = Polarity[ch] * (Waveforms[ch][sample] - BaselineValue[ch]);
	      TriggerHeight = Polarity[ch] * (TheSettings->ChTriggerThreshold[ch] - BaselineValue[ch]);
	      
	      // Simple algorithm to determine the pulse height (adc)
	      // and peak position (sample) by looping over all samples
	      if(SampleHeight > PulseHeight){
		PulseHeight = SampleHeight;
		PeakPosition = sample;
	      }
	      
	      // Integrate the area under the pulse
	      if(SampleHeight > TriggerHeight)
		PulseArea += SampleHeight;
	    }
	  }
	} // End sample loop


	/////////////////////////////////
	// Post-sampling pulse processing

	// Compute the trigger time stamp for the waveform
	TimeStamp = EventInfo.TriggerTimeTag;

	// Because the PSD integrals are computed relative to the peak
	// position durong on-the-fly processing, we must take fast
	// PSD integrals after sampling has concluded. We will ONLY do
	// this if some aspect of PSD is desired by user for efficiency.
	
	if(TheSettings->PSDMode or TheSettings->WaveformStorePSDData){
	  Int_t sample = TheSettings->ChPSDTotalStart[ch];
	  for(; sample<TheSettings->ChPSDTotalStop[ch]; sample++)
	    PSDTotal += Waveforms[ch][sample];
	  
	  sample = TheSettings->ChPSDTailStart[ch];
	  for(; sample<TheSettings->ChPSDTailStop[ch]; sample++)
	    PSDTail += Waveforms[ch][sample];
	}
	  
	if(!TheSettings->DisplayNonUpdateable){
	  
	  // Fill the channel-specific ADAQWaveformData objects with
	  // the processed waveform data. Note that this is only done
	  // for non-ultra rate modes since it requires analysis of
	  // the waveforms.
	  
	  WaveformData[ch]->SetPulseHeight(PulseHeight);
	  WaveformData[ch]->SetPulseArea(PulseArea);
	  WaveformData[ch]->SetBaseline(BaselineValue[ch]);
	  WaveformData[ch]->SetPSDTotalIntegral(PSDTotal);
	  WaveformData[ch]->SetPSDTailIntegral(PSDTail);
	  WaveformData[ch]->SetTimeStamp(TimeStamp);
	  WaveformData[ch]->SetChannelID(ch);
	  WaveformData[ch]->SetBoardID(DGManager->GetBoardID());

	  // Compute and store the absolute sample numbers
	  // corresponding to the PSD total/tail integration limits
	  // based on the calculated peak position sample
	  
	  if(TheSettings->PSDMode or TheSettings->DisplayPSDLimits){
	    PSDTotalAbsStart[ch] = PeakPosition + TheSettings->ChPSDTotalStart[ch];
	    PSDTotalAbsStop[ch] = PeakPosition + TheSettings->ChPSDTotalStop[ch];
	    PSDTailAbsStart[ch] = PeakPosition + TheSettings->ChPSDTailStart[ch];
	    PSDTailAbsStop[ch] = PeakPosition + TheSettings->ChPSDTailStop[ch];
	  }
	  
	  // Use the calibration curves (ROOT TGraph's) to convert
	  // the pulse height/area from ADC to the user's energy
	  // units using linear interpolation calibration points
	  if(CalibrationEnable[ch]){
	    
	    if(TheSettings->SpectrumPulseHeight)
	      PulseHeight = CalibrationCurves[ch]->Eval(PulseHeight);
	    else
	      PulseArea = CalibrationCurves[ch]->Eval(PulseArea);
	  }

	  if(TheSettings->SpectrumMode){
	    
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

	  else if(TheSettings->PSDMode){
	    PSDHistogram_H[ch]->Fill(PSDTotal, PSDTail);
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
	
	// Fill the waveform tree via the readout manager
	
	if(TheSettings->WaveformStoreRaw)
	  TheReadoutManager->GetWaveformTree()->Fill();
	
	if(TheSettings->WaveformStoreEnergyData or TheSettings->WaveformStorePSDData)
	  TheReadoutManager->GetWaveformDataTree()->Fill();
	
	// Reset the bool used to determine if the LLD/ULD window
	// should be used as the "trigger" for writing waveforms
	FillWaveformTree = false;
      }
      
      // Plot the waveform; only plot the first waveform event in the
      // case of a multievent readout to maximize efficiency
      
      if(TheSettings->WaveformMode and evt == 0){
	// Draw the digitized waveform
	TheGraphicsManager->PlotWaveforms(Waveforms, WaveformLength);
	
	// Draw graphical objects associated with the waveform
	TheGraphicsManager->DrawWaveformGraphics(BaselineValue,
						 PSDTotalAbsStart,
						 PSDTotalAbsStop,
						 PSDTailAbsStart,
						 PSDTailAbsStop);
      }
    }// End readout event loop
    
    // Only if the display is set to continuous then plot the spectra
    // or PSD histogram; if the display mode is set to updateable the
    // plotting is achieved by clicking the "Update Display" text
    // button beneath the canvas. Plotting is disabled if the display
    // mode is in nonupdateable mode.

    if(TheSettings->DisplayContinuous){

      if(TheSettings->SpectrumMode){
	Int_t Entries = Spectrum_H[TheSettings->SpectrumChannel]->GetEntries();
	Int_t Rate = TheSettings->SpectrumRefreshRate;
	if(Entries % Rate == 0)
	  TheGraphicsManager->PlotSpectrum(Spectrum_H[TheSettings->SpectrumChannel]);
      }
      
      else if(TheSettings->PSDMode){
	Int_t Entries = Spectrum_H[TheSettings->SpectrumChannel]->GetEntries();
	Int_t Rate = TheSettings->SpectrumRefreshRate;
	if(Entries % Rate == 0)
	  TheGraphicsManager->PlotPSDHistogram(PSDHistogram_H[TheSettings->PSDChannel]);
      }
    }
  } // End acquisition loop

  // Free the memory preallocated for event readout
  DGManager->FreeEvent(&EventWaveform);
  
  // Free the PC memory allocated to the readout buffer
  DGManager->FreeReadoutBuffer(&Buffer);
}


void AAAcquisitionManager::StopAcquisition()
{
  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();
  Int_t AcqControl = TheSettings->AcquisitionControl;
  if(AcqControl == 0)
    DGManager->SWStopAcquisition();
  else if(AcqControl == 1 or AcqControl == 2)
    DGManager->SInDisarmAcquisition();
  
  AcquisitionEnable = false;
  
  DGManager->FreeEvent(&EventWaveform);
  DGManager->FreeReadoutBuffer(&Buffer);
  
  if(AcquisitionTimerEnable){

    // Set the information in the ADAQ file to signal that the
    // acquisition timer was used for this run and set the acquisition
    // time. This must be done here at the end of acquisition Since
    // when the ADAQReadoutInformation class is filled - when the file
    // _created_ not when the acquisition timer button is pushed - the
    // acquisition timer boolean has not yet been set.
    
    ADAQReadoutInformation *ARI = TheReadoutManager->GetReadoutInformation();
    ARI->SetAcquisitionTimer(AcquisitionTimerEnable);
    ARI->SetAcquisitionTime(TheSettings->AcquisitionTime);
    
    AcquisitionTimerEnable = false;
    TheInterface->UpdateAfterAQTimerStopped(ADAQFileIsOpen);
  }
  
  if(ADAQFileIsOpen)
    CloseADAQFile();
}


void AAAcquisitionManager::SaveObjectData(string ObjectType,
					  string FileName)
{
  Int_t Channel = TheSettings->SpectrumChannel;
  
  if(!SpectrumExists[Channel] or !PSDHistogramExists[Channel])
    return;
  
  size_t Found = FileName.find_last_of(".");
  if(Found != string::npos){

    // Get the file extension
    string FileExtension = FileName.substr(Found, string::npos);
    
    // The object (waveform, spectrum, PSD histogram) will be output
    // in a form factor that depends on the file extension:
    //   .dat: A columnar ASCII file
    //   .csv: a columnar CSV file
    //   .root: ROOT object is written to disk with the name:
    //          -> "Waveform" - Digitized waveform (TH1F)
    //          -> "Spectrum" - Pulse spectrum (TH1F)
    //          -> "PSDHistogram" - PSD histogram (TH2D)

    if(FileExtension == ".dat" or FileExtension == ".csv"){
      
      if(TheSettings->ObjectSaveWithTimeExtension){
	time_t Time = time(NULL);
	stringstream SS;
	SS << "." << Time;
	string TimeString = SS.str();
	
	FileName.insert(Found, TimeString);
      }
      
      // Create an ofstream object to write the data to a file
      ofstream ObjectOutput(FileName.c_str(), ofstream::trunc);
      
      // Assign the data separator based on file extension
      string Separator;
      if(FileExtension == ".dat")
	Separator = "\t";
      else if(FileExtension == ".csv")
	Separator = ",";

      if(ObjectType == "Waveform"){
	cout << "\nAAAcquisitionManager::SaveObjectData() : Waveform output is not yet implemented!\n"
	     << endl;
      }
      else if(ObjectType == "Spectrum"){
	
	// Get the number of bins in the spectrum histogram
	Int_t NumBins = Spectrum_H[Channel]->GetNbinsX();
	
	// Iterate over all the bins in spectrum histogram and output
	// the bin center (value on the X axis of the histogram) and the
	// bin content (value on the Y axis of the histogram)
	for(Int_t bin=1; bin<=NumBins; bin++){
	  Double_t BinCenter = Spectrum_H[Channel]->GetBinCenter(bin);
	  Double_t BinContent = Spectrum_H[Channel]->GetBinContent(bin);
	  
	  ObjectOutput << BinCenter << Separator << BinContent
		       << endl;
	}
      }
      else if(ObjectType == "PSDHistogram"){
	Int_t NumXBins = PSDHistogram_H[Channel]->GetNbinsX();
	Int_t NumYBins = PSDHistogram_H[Channel]->GetNbinsY();

	Int_t XMin = PSDHistogram_H[Channel]->GetXaxis()->GetXmin();
	Int_t XMax = PSDHistogram_H[Channel]->GetXaxis()->GetXmax();
	Int_t YMin = PSDHistogram_H[Channel]->GetYaxis()->GetXmin();
	Int_t YMax = PSDHistogram_H[Channel]->GetYaxis()->GetXmax();

	cout << setw(10) << NumXBins << setw(10) << XMin << setw(10) << XMax << "\n"
	     << setw(10) << NumYBins << setw(10) << YMin << setw(10) << YMax << "\n"
	     << endl;
	
	for(Int_t xbin=1; xbin<=NumXBins; xbin++){
	  for(Int_t ybin=1; ybin<=NumYBins; ybin++){
	    Int_t Bin = NumYBins*xbin + ybin;
	    ObjectOutput << PSDHistogram_H[Channel]->GetBinContent(Bin)
			 << endl;
	  }
	}
      }
      
      // Close the ofstream object
      ObjectOutput.close();
    }
    else if(FileExtension == ".root"){

      TFile *ObjectOutput = new TFile(FileName.c_str(), "recreate");

      if(ObjectType == "Waveform"){
      }
      
      else if(ObjectType == "Spectrum")
	Spectrum_H[Channel]->Write("Spectrum");
      
      else if(ObjectType == "PSDHistogram")
	PSDHistogram_H[Channel]->Write("PSDHistogram");
      
      ObjectOutput->Close();
    }
  }
}


Bool_t AAAcquisitionManager::AddCalibrationPoint(Int_t Channel, Int_t SetPoint,
						 Double_t Energy, Double_t PulseUnit)
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


Bool_t AAAcquisitionManager::EnableCalibration(Int_t Channel)
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
    const Int_t NumPoints = CalibrationData[Channel].PointID.size();
    
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


Bool_t AAAcquisitionManager::ResetCalibration(Int_t Channel)
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


Bool_t AAAcquisitionManager::LoadCalibration(Int_t Channel, 
					   string FileName, 
					   Int_t &NumPoints)
{
  size_t Found = FileName.find_last_of(".");
  if(Found != string::npos){
    
    // Set the calibration file to an input stream
    ifstream In(FileName.c_str());
    
    // An index to control the set point
    Int_t SetPoint = 0;
    
    // Iterate through each line in the file and use the values
    // (column1 == energy, column2 == pulse unit) to set the
    // CalibrationData objects for the current channel
    while(In.good()){
      Double_t Energy, PulseUnit;
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


Bool_t AAAcquisitionManager::WriteCalibration(Int_t Channel, string FileName)
{
  // Set the calibration file to an input stream
  ofstream Out(FileName.c_str());
  
  for(Int_t i=0; i<CalibrationData[Channel].Energy.size(); i++)
    Out << setw(10) << CalibrationData[Channel].Energy[i]
	<< setw(10) << CalibrationData[Channel].PulseUnit[i]
	<< endl;
  Out.close();

  return true;
}


void AAAcquisitionManager::CreateADAQFile(string FileName)
{
  if(TheReadoutManager->GetADAQFileOpen())
    return;
  
  // Create a new ADAQ file via the readout manager
  TheReadoutManager->CreateFile(FileName);

  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();

  // Create two branches for each digitizer channel:
  // - A branch on the waveform TTree for raw waveform readout
  // - A branch on the waveform data TTree for analyzed waveform data readout

  Int_t DGChannels = DGManager->GetNumChannels();
  for(Int_t ch=0; ch<DGChannels; ch++){
    TheReadoutManager->CreateWaveformTreeBranch(ch, &Waveforms[ch]);
    TheReadoutManager->CreateWaveformDataTreeBranch(ch, WaveformData[ch]);
  }
  
  // Get the pointer to the ADAQ readout information and fill with all
  // relevent information via the ADAQReadoutInformation::Set*() methods
  
  ADAQReadoutInformation *ARI = TheReadoutManager->GetReadoutInformation();
  
  // Set physical information about the digitizer device
  ARI->SetDGModelName      (DGManager->GetBoardModelName() );
  ARI->SetDGSerialNumber   (DGManager->GetBoardSerialNumber() );
  ARI->SetDGNumChannels    (DGManager->GetNumChannels() );
  ARI->SetDGBitDepth       (DGManager->GetNumADCBits() );
  ARI->SetDGSamplingRate   (DGManager->GetSamplingRate() );
  ARI->SetDGROCFWRevision  (DGManager->GetBoardROCFirmwareRevision() );
  ARI->SetDGAMCFWRevision  (DGManager->GetBoardAMCFirmwareRevision() );

  // Fill the global acquisition settings
  ARI->SetRecordLength         (TheSettings->RecordLength );
  ARI->SetPostTrigger          (TheSettings->PostTrigger );
  ARI->SetCoincidenceLevel     (TheSettings->TriggerCoincidenceLevel );
  ARI->SetDataReductionMode    (TheSettings->DataReductionEnable );
  ARI->SetZeroSuppressionMode  (TheSettings->ZeroSuppressionEnable );
  ARI->SetTriggerType          (TheSettings->TriggerTypeName );
  ARI->SetTriggerEdge          (TheSettings->TriggerEdgeName );
  ARI->SetAcquisitionType      (TheSettings->AcquisitionControlName );
  
  // Fill the channel specific settings
  ARI->SetTrigger          (TheSettings->ChTriggerThreshold );
  ARI->SetBaselineCalcMin  (TheSettings->ChBaselineCalcMin );
  ARI->SetBaselineCalcMax  (TheSettings->ChBaselineCalcMax );
  ARI->SetChannelEnable    (TheSettings->ChEnable );
  ARI->SetDCOffset         (TheSettings->ChDCOffset );
  ARI->SetZLEFwd           (TheSettings->ChZLEForward );
  ARI->SetZLEBck           (TheSettings->ChZLEBackward );
  ARI->SetZLEThreshold     (TheSettings->ChZLEThreshold );

  // Fill information regarding waveform acquisition
  ARI->SetStoreRawWaveforms  (TheSettings->WaveformStoreRaw);
  ARI->SetStoreEnergyData    (TheSettings->WaveformStoreEnergyData);
  ARI->SetStorePSDData       (TheSettings->WaveformStorePSDData);
}


void AAAcquisitionManager::CloseADAQFile()
{
  if(!TheReadoutManager->GetADAQFileOpen())
    return;
  
  TheReadoutManager->WriteFile();
}


/*
void AAInterface::GenerateArtificialWaveform(Int_t RecordLength, vector<int> &Voltage, 
						 Double_t *Voltage_graph, Double_t VerticalPosition)
{
  // Exponential time constants with small random variations
  const Double_t t1 = 20. - (RNG->Rndm()*10);
  const Double_t t2 = 80. - (RNG->Rndm()*40);;
  
  // The waveform amplitude with small random variations
  const Double_t a = RNG->Rndm() * 30;
  
  // Set an artificial baseline for the pulse
  const Double_t b = 3200;
  
  // Set an artifical polarity for the pulse
  const Double_t p = -1.0;
  
  // Set the number of leading zeros before the waveform begins with
  // small random variations
  const Int_t NumLeadingSamples = 100;
  
  // Fill the Voltage vector with the artificial waveform
  for(Int_t sample=0; sample<RecordLength; sample++){
    
    if(sample < NumLeadingSamples){
      Voltage[sample] = b + VerticalPosition;
      Voltage_graph[sample] = b + VerticalPosition;
    }
    else{
      Double_t t = (sample - NumLeadingSamples)*1.0;
      Voltage[sample] = (p * (a * (t1-t2) * (exp(-t/t1)-exp(-t/t2)))) + b + VerticalPosition;
      Voltage_graph[sample] = Voltage[sample];
    }
  }
}
*/
