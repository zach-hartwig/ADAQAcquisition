#include "ADAQDigitizer.hh"

#include "AASubtabSlots.hh"
#include "AAInterface.hh"

AASubtabSlots::AASubtabSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AASubtabSlots::~AASubtabSlots()
{;}


void AASubtabSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  switch(ActiveID){

    // Enable the calibration widgets
  case SpectrumCalibration_CB_ID:

    if(TI->DGScopeSpectrumCalibration_CB->IsDown())
      TI->SetCalibrationWidgetState(true, kButtonUp);
    else
      TI->SetCalibrationWidgetState(false, kButtonDisabled);

    break;
  }
}


void AASubtabSlots::HandleComboBoxes(int ActivexID, int SelectedID)
{
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;

  /*
  switch(ActiveID){
    
  case DGScopeSpectrumChannel_CBL_ID:{

    // In order to ensure that the calibration set point combo box
    // displays the correct number of calibration points, we need to
    // rebuild the combo box entry list each time the user selects a
    // new histogram channel to plot using this combo box

    // Get the number of calibration points for the current histogram
    // (SelectedID == histogram channel for this combo box)
    int LastPointIndex = CalibrationData[SelectedID].PointID.size();

    // Remove the previous entries from the combo box
    int TotalEntries = DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->GetNumberOfEntries();
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->RemoveEntries(0,TotalEntries-1);

    // Recreate the entries for each calibration point in the present
    // histogram calibration combo box
    stringstream ss;
    for(int i=0; i<=LastPointIndex; i++){
      ss << "Calibration point " << i;
      string Entry = ss.str();
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(Entry.c_str(), i);
      ss.str("");
    }

    // If the current histogram has calibration points then set the
    // combo box to diplay the to-be-set point
    if(LastPointIndex > 0){
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(LastPointIndex);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }

    // ... otherwise display the no-calibration points values
    else{
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(0);
    }
    break;
  }

  case DGScopeSpectrumCalibrationPoint_CBL_ID:{

    int SpectrumChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();    
    int Size = CalibrationData[SpectrumChannel].PointID.size();

    if(SelectedID <= (Size-1)){

      double Energy = CalibrationData[SpectrumChannel].Energy.at(SelectedID);
      double PulseUnit = CalibrationData[SpectrumChannel].PulseUnit.at(SelectedID);

      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(Energy);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(PulseUnit);

      double Value = Energy/DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();

      DGScopeHorizontalScale_THS->SetPointerPosition(Value);
    }
    else{
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }
    break;
  }
  }
  */
}


void AASubtabSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveID = ActiveEntry->WidgetId();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){
    
  case SpectrumCalibrationEnergy_NEL_ID:
  case SpectrumCalibrationPulseUnit_NEL_ID:{
    double Value = 0.;
    if(ActiveID == SpectrumCalibrationEnergy_NEL_ID)
      Value = TI->DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();
    else
      Value = TI->DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    
    // Normalize value for slider position setting from 0 to 1
    Value /= TI->DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();
    
    TI->DisplayHorizontalScale_THS->SetPointerPosition(Value);
    break;
  }
  }
}


void AASubtabSlots::HandleTextButtons()
{
  TGTextButton *ActiveButton = (TGTextButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){

    //////////////////
    // Acquisition tab 

  case AQTimerStart_TB_ID:{
    
    if(TI->AQStartStop_TB->GetString() != "Acquiring")
      break;
    
    if(ActiveButton->GetString() == "Start timer"){
      
      // Set the graphical attributes of the text button
      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      ActiveButton->SetText("Waiting ...");
      
      // Get the start time (i.e. now)
      TheVMEManager->SetAcquisitionTimeStart( time(NULL) );

      // Get the stop time (i.e. amount of time to run in seconds)
      TheVMEManager->SetAcquisitionTimeStop( TI->AQTime_NEL->GetEntry()->GetNumber() );

      // Set the bool that will trigger the check of the timer against
      // the current time within the acquisition loop in RunDGScope()
      TheVMEManager->SetAcquisitionTimerEnable(true);

      // If the ROOT data file is open but the user has not enabled
      // data storage, assume that the user wants to acquire data for
      // the specific amount of time dictated by the acquisition timer
      //if(ROOTFileOpen){
      //	if(OutputDataFile->IsOpen() and !DGScopeDataStorageEnable_CB->IsDown())
      //	  DGScopeDataStorageEnable_CB->SetState(kButtonDown);
      //      }
    }
    break;
  }

  case AQTimerAbort_TB_ID:{
    if(TheVMEManager->GetDGAcquisitionEnable() and
       TheVMEManager->GetAcquisitionTimerEnable())
      {}//StopAcquisitionSafely();
    break;
  }

  case CheckBufferStatus_TB_ID:{

    int DGChannels = TheVMEManager->GetDGManager()->GetNumChannels();

    bool BufferStatus[DGChannels];
    for(int ch=0; ch<DGChannels; ch++)
      BufferStatus[ch] = false;
    
    TheVMEManager->GetDGManager()->CheckBufferStatus(BufferStatus);
    
    bool BufferFull = false;
    
    for(int ch=0; ch<DGChannels; ch++){
      if(BufferStatus[ch] == true)
	BufferFull = true;
    }
    
    if(BufferFull)
      TI->DGBufferStatus_TE->SetText("Buffers are FULL");
    else
      TI->DGBufferStatus_TE->SetText("Buffers are OK");
    
    break;
  }

    ///////////////
    // Spectrum tab

    /////////////////////////////////////////////////
    // Add a new DGScope spectrum calibration point
  case SpectrumCalibrationSetPoint_TB_ID:{

    // Get the calibration point to be set
    uint SetPoint = TI->DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->GetSelected();

    // Get the energy of the present calibration point
    double Energy = TI->DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();

    // Get the pulse unit value of the present calibration point
    int PulseUnit = TI->DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = TI->DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

    /*
    if(SetPoint == CalibrationData[CurrentChannel].PointID.size()){

      CalibrationData[CurrentChannel].PointID.push_back(SetPoint);
      CalibrationData[CurrentChannel].Energy.push_back(Energy);
      CalibrationData[CurrentChannel].PulseUnit.push_back(PulseUnit);
      
      // Add a new point to the number of calibration points in case
      // the user wants to add subsequent points to the calibration
      stringstream ss;
      ss << (SetPoint+1);
      string NewPointLabel = "Calibration point " + ss.str();
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(NewPointLabel.c_str(),SetPoint+1);
      
      // Set the combo box to display the new calibration point...
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(SetPoint+1);
      
      // ...and set the calibration energy and pulse unit ROOT number
      // entry widgets to their default "0.0" and "1.0" respectively,
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);
    }

    // ...or if the user is re-setting previously set points then
    // simply overwrite the preexisting values in the vectors
    else{
      CalibrationData[CurrentChannel].Energy[SetPoint] = Energy;
      CalibrationData[CurrentChannel].PulseUnit[SetPoint] = PulseUnit;
    }
    break;
    */
  }

    /*


    ///////////////////////////////////////////////
    // Create a new DGScope spectrum calibration
  case DGScopeSpectrumCalibrationCalibrate_TB_ID:{

    // If there are 2 or more points in the current channel's
    // calibration data set then create a new TGraph object. The
    // TGraph object will have pulse units [ADC] on the X-axis and the
    // corresponding energies [in whatever units the user has entered
    // the energy] on the Y-axis. A TGraph is used because it provides
    // very easy but extremely powerful methods for interpolation,
    // which allows the pulse height/area to be converted in to energy
    // efficiently in the acquisition loop.

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(CalibrationData[CurrentChannel].PointID.size() >= 2){

      // Determine the total number of calibration points in the
      // current channel's calibration data set
      const int NumCalibrationPoints = CalibrationData[CurrentChannel].PointID.size();

      // Create a new "CalibrationManager" TGraph object.
      CalibrationManager[CurrentChannel] = new TGraph(NumCalibrationPoints,
						      &CalibrationData[CurrentChannel].PulseUnit[0],
						      &CalibrationData[CurrentChannel].Energy[0]);
      
      // Set the current channel's calibration boolean to true,
      // indicating that the current channel will convert pulse units
      // to energy within the acquisition loop before histogramming
      // the result into the channel's spectrum
      UseCalibrationManager[CurrentChannel] = true;


      DGScopeSpectrumCalibrationCalibrate_TB->SetText("Calibrated");
      DGScopeSpectrumCalibrationCalibrate_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
      DGScopeSpectrumCalibrationCalibrate_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
    }
    break;
  }
    
  case DGScopeSpectrumCalibrationPlot_TB_ID:{
    
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(UseCalibrationManager[CurrentChannel]){
      TCanvas *Calibration_C = new TCanvas("Calibration_C","CalibrationManager TGraph",0,0,600,400);
      Calibration_C->SetLeftMargin(0.14);
      Calibration_C->SetBottomMargin(0.14);
      
      stringstream ss;
      ss << "CalibrationManager TGraph for Channel[" << CurrentChannel << "]";
      string Title = ss.str();

      CalibrationManager[CurrentChannel]->SetTitle(Title.c_str());
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitle("Pulse unit [ADC]");
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleOffset(1.1);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetNdivisions(505);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitle("Energy");
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleOffset(1.2);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->SetMarkerSize(2);
      CalibrationManager[CurrentChannel]->SetMarkerStyle(22);
      CalibrationManager[CurrentChannel]->Draw("ALP");

      Calibration_C->Update();

      DGScope_EC->GetCanvas()->cd(0);
    }
    break;
  }

  case DGScopeSpectrumCalibrationReset_TB_ID:{

    // Get the current channel being histogrammed in DGScope
    int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

    // Clear the channel calibration vectors for the current channel
    CalibrationData[CurrentChannel].PointID.clear();
    CalibrationData[CurrentChannel].Energy.clear();
    CalibrationData[CurrentChannel].PulseUnit.clear();

    // Delete the current channel's depracated calibration manager
    // TGraph object to prevent memory leaks
    if(UseCalibrationManager[CurrentChannel])
      delete CalibrationManager[CurrentChannel];

    // Set the current channel's calibration boolean to false,
    // indicating that the calibration manager will NOT be used within
    // the acquisition loop
    UseCalibrationManager[CurrentChannel] = false;
    
    // Reset the calibration widgets
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->RemoveAll();
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0", 0);
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
    DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
    DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);

    DGScopeSpectrumCalibrationCalibrate_TB->SetText("Calibrate");
    DGScopeSpectrumCalibrationCalibrate_TB->SetForegroundColor(ColorManager->Number2Pixel(1));
    DGScopeSpectrumCalibrationCalibrate_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
    
    break;
  }

  case DGScopeSpectrumCalibrationLoad_TB_ID:{

    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};

    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &FileInformation);

    if(FileInformation.fFilename==NULL)
      {}

    else{
      
      int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

      string CalibrationFileName = FileInformation.fFilename;

      size_t Found = CalibrationFileName.find_last_of(".");
      if(Found != string::npos){

	// Set the calibration file to an input stream
	ifstream In(CalibrationFileName.c_str());

	// Reset any preexisting calibrations
	DGScopeSpectrumCalibrationReset_TB->Clicked();

	// An index to control the set point
	int SetPoint = 0;

	// Iterate through each line in the file and use the values
	// (column1 == energy, column2 == pulse unit) to set the
	// CalibrationData objects for the current channel
	while(In.good()){
	  double Energy, PulseUnit;
	  In >> Energy >> PulseUnit;

	  if(In.eof()) break;

	  CalibrationData[CurrentChannel].PointID.push_back(SetPoint);
	  CalibrationData[CurrentChannel].Energy.push_back(Energy);
	  CalibrationData[CurrentChannel].PulseUnit.push_back(PulseUnit);

	  // Add a new point to the number of calibration points in case
	  // the user wants to add subsequent points to the calibration
	  stringstream ss;
	  ss << (SetPoint+1);
	  string NewPointLabel = "Calibration point " + ss.str();
	  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(NewPointLabel.c_str(),SetPoint+1);
	  
	  // Set the combo box to display the new calibration point...
	  DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(SetPoint+1);

	  SetPoint++;
	}

	// Use the loaded calibration points to set the calibration
	DGScopeSpectrumCalibrationCalibrate_TB->Clicked();
      }
    }
    break;
  }

  case DGScopeSpectrumCalibrationWrite_TB_ID:{
    
    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};
    
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    if(FileInformation.fFilename==NULL)
      {}

    else{
      
      int CurrentChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();

      string CalibrationFileName = FileInformation.fFilename;

      // Set the calibration file to an input stream
      ofstream Out(CalibrationFileName.c_str());
      
      for(int i=0; i<CalibrationData[CurrentChannel].Energy.size(); i++)
	Out << setw(10) << CalibrationData[CurrentChannel].Energy[i]
	    << setw(10) << CalibrationData[CurrentChannel].PulseUnit[i]
	    << endl;
      Out.close();
    }
    break;
  }



    //////////////////////////
    // Save DGScope graphics
  case DGScopeSave_TB_ID:{
    
    // Obtain the base name (no "." or file extension) from the ROOT text entry widget
    string fBaseName = DGScopeDisplayOutputFileName_TEL->GetEntry()->GetText();

    // Obtain the file type and extension from the ROOT combo box widget
    string fExtension = "";
    int PlotType = DGScopeDisplayOutputFileType_CBL->GetComboBox()->GetSelected();
    switch(PlotType){
    case 0:
      fExtension.assign(".eps");
      break;
    case 1:
      fExtension.assign(".ps");
      break;
    case 2:
      fExtension.assign(".png");
      break;
    case 3:
      fExtension.assign(".jpg");
      break;
    }

    // Perform a check to ensure that the specified total file name
    // does not exist to prevent overwriting important graphics; if
    // the file does exist, increment an integer counter that is added
    // onto the file num to create a new but recognizable file name
    int num = 0;
    stringstream ss;
    string fNumber;
    string fName;

    bool fExists = true;
    while(fExists){
	  ss << num;
	  fNumber.assign(ss.str());
	  fName = fBaseName+fNumber+fExtension;
	  ifstream fCheck(fName.c_str());
	  fExists = (bool)fCheck;
	  ss.str("");
	  num++;
    }

    // Print the state of the DGScope embedded canvas to file
    DGScope_EC->GetCanvas()->Print(fName.c_str(), fExtension.c_str());
    break;
  }



    //////////////////////////////
    // Set the ROOT data file name

  case DGScopeDataFileName_TB_ID:{
    
    const char *FileTypes[] = {"ADAQ ROOT file","*.adaq",
			       "ADAQ ROOT file","*.root",
			       0, 0};
    
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fOverwrite = false;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);
    
    if(FileInformation.fFilename==NULL)
      {}
    else{
      DataFileName = FileInformation.fFilename;
      
      size_t Found = DataFileName.find_last_of(".");
      if(Found != string::npos)
	DataFileName = DataFileName.substr(0, Found);

      DataFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];

      Found = DataFileExtension.find_last_of("*");
      DataFileExtension = DataFileExtension.substr(Found+1, DataFileExtension.size());
      string FileName_StripPath = DataFileName + DataFileExtension;

      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());

      DGScopeDataFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    
    break;
  }

    ///////////////////////////
    // Create ROOT data file
  case DGScopeDataStorageCreateFile_TB_ID:{

    ///////////////////////////////////////////////
    // Test to ensure data file is not already open


    /////////////////////////////////////////////
    // Instantiate objects for persistent storage

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
    
    
    /////////////////////////////////////////////
    // Retrieve all values (except the waveforms)
    
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
    
    ////////////////////////////
    // Set the infamous booleans
    
    // Set a bool indicating that the next digitized event will
    // trigger the creation of a TTree branch with the correctly sized
    // array. This action is performed once in
    // AAInterface::RunDGScope(). See that function for more comments
    BranchWaveformTree = true;

    ROOTFileOpen = true;
    
    //////////////////////////////////
    // Set widget states appropriately
    
    // Disable the filename, comment, and create file button (since we
    // don't want to create new ROOT files until the active is closed)
    // and activate the close file and enable buttons (since these
    // options are now available with an open ROOT file for data writing)
    //DGScopeDataFileName_TEL->GetEntry()->SetState(false);
    //DGScopeDataComment_TEL->GetEntry()->SetState(false);
    DGScopeDataStorageCreateFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(8));
    DGScopeDataStorageCreateFile_TB->SetText("ADAQ file created");
    DGScopeDataStorageCloseFile_TB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
  
    break;
  }

    ///////////////////////////////
    // Write and close ROOT file
  case DGScopeDataStorageCloseFile_TB_ID:{
    
    if(!ROOTFileOpen)
      break;
    
    if(DGScopeDataStorageEnable_CB->IsDown())
      DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    
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
    delete OutputDataFile;

    // Set widget states appropriately.
    DGScopeDataStorageCreateFile_TB->SetState(kButtonUp);
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(ColorManager->Number2Pixel(18));
    DGScopeDataStorageCreateFile_TB->SetText("Create ADAQ file");
    DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);

    ROOTFileOpen = false;

    break;
  }

  case DGScopeSpectrumFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"Space-separated format", ".dat",
			       "Comma-separated format", ".csv",
			       0, 0};
    
    // Create a new window containing the file save dialog. Set the
    // default directory to the user's present directory
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    // If a file name was NOT successfully created ...
    if(FileInformation.fFilename==NULL)
      {}
    
    // If a file name was successfully created ...
    else{
      // Get the file name from the text entry widget
      SpectrumFileName = FileInformation.fFilename;
      
      // If the user entered the file name with a dot followed by a
      // file extension, strip the them from the file name since the
      // spectrum file type is assigned via the combo box selection
      // from the file dialog. This ensures that regardless of the
      // user's entry, the right format will prevail!
      size_t Found = SpectrumFileName.find_last_of(".");
      if(Found != string::npos)
	SpectrumFileName = SpectrumFileName.substr(0, Found);
      
      // Get the file type enxtension from the combo box widget (note
      // the "+1" required to get the index right)
      SpectrumFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];
      
      // Get just the file name + file extension by stripping off the
      // absolute path from the full file name

      string FileName_StripPath = SpectrumFileName + SpectrumFileExtension;
      
      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());
      
      DGScopeSpectrumFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    break;
  }
    
  case DGScopeSaveSpectrum_TB_ID:{
    SaveSpectrumData();
    break;
  }
    
  case DGScopeCanvasFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"EPS file", ".eps",
			       "PS file", ".ps",
			       "PDF file", ".pdf",
			       "PNG file", ".png",
			       "JPG file", ".jpeg",
			       0, 0};
    
    // Create a new window containing the file save dialog. Set the
    // default directory to the user's present directory
    TGFileInfo FileInformation;
    FileInformation.fFileTypes = FileTypes;
    FileInformation.fIniDir = StrDup(getenv("PWD"));
    new TGFileDialog(gClient->GetRoot(), this, kFDSave, &FileInformation);

    // If a file name was NOT successfully created ...
    if(FileInformation.fFilename==NULL)
      {}//CreateMessageBox("No file name was selected! The canvas graphics will not be saved!","Stop");
    
    // If a file name was successfully created ...
    else{
      // Get the file name from the text entry widget
      GraphicsFileName = FileInformation.fFilename;
      
      // Get the file type enxtension from the combo box widget (note
      // the "+1" required to get the index right)
      GraphicsFileExtension = FileInformation.fFileTypes[FileInformation.fFileTypeIdx+1];

      // If the user did not enter the file name with the extension,
      // tack it on to the end
      size_t Found = GraphicsFileName.find_last_of(".");
      if(Found == string::npos)
	GraphicsFileName = GraphicsFileName.substr(0,Found) + GraphicsFileExtension;

      string FileName_StripPath = GraphicsFileName;
      Found = FileName_StripPath.find_last_of("/");
      if(Found != string::npos)
	FileName_StripPath = FileName_StripPath.substr(Found+1, FileName_StripPath.size());
      
      DGScopeCanvasFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
    }
    break;
  }
    
    
  case DGScopeSaveCanvas_TB_ID:{
    
    string FileName;
    
    if(DGScopeSaveCanvasWithTimeExtension_CB->IsDown()){
      time_t CurrentTime = time(NULL);
      stringstream ss;
      ss << "." << CurrentTime;
      string CurrentTimeString = ss.str();
      FileName = GraphicsFileName + CurrentTimeString;
    }
    else
      FileName = GraphicsFileName;
    
    DGScope_EC->GetCanvas()->Print(FileName.c_str(), GraphicsFileExtension.c_str());
    break;
  }
    
  */
  }
}
  

void AASubtabSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  switch(ActiveID){
    
  case SpectrumAnalysisHeight_RB_ID:
    
    if(TI->DGScopeSpectrumAnalysisHeight_RB->IsDown()){
      TI->DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(0);
      TI->DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(4095);
    }

    break;

  case SpectrumAnalysisArea_RB_ID:
    
    if(TI->DGScopeSpectrumAnalysisArea_RB->IsDown()){
      TI->DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(4000);
      TI->DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(100000);
    }

    break;
  }
}
