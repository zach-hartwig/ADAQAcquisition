#include <TGFileDialog.h>

#include <sstream>
#include <fstream>

#include "ADAQDigitizer.hh"

#include "AASubtabSlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"

AASubtabSlots::AASubtabSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AASubtabSlots::~AASubtabSlots()
{;}


void AASubtabSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();
  
  switch(ActiveID){

    // Enable the calibration widgets
  case SpectrumCalibration_CB_ID:
    
    if(TI->SpectrumCalibration_CB->IsDown())
      TI->SetCalibrationWidgetState(true, kButtonUp);
    else
      TI->SetCalibrationWidgetState(false, kButtonDisabled);
    
    break;
    
  case DGTriggerCoincidenceEnable_CB_ID:
    
    if(TI->DGTriggerCoincidenceEnable_CB->IsDown())
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(true);
    else
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(false);

    break;
  }
}


void AASubtabSlots::HandleComboBoxes(int ActiveID, int SelectedID)
{
  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;
  
  switch(ActiveID){
    
  case SpectrumChannel_CBL_ID:{

    // In order to ensure that the calibration set point combo box
    // displays the correct number of calibration points, we need to
    // rebuild the combo box entry list each time the user selects a
    // new histogram channel to plot using this combo box

    // Get the number of calibration points for the current histogram
    // (SelectedID == histogram channel for this combo box)
    
    int LastPointIndex = AAAcquisitionManager::GetInstance()->
      GetCalibrationDataStruct(SelectedID).PointID.size();

    // Remove the previous entries from the combo box
    int TotalEntries = TI->SpectrumCalibrationPoint_CBL->GetComboBox()->GetNumberOfEntries();
    TI->SpectrumCalibrationPoint_CBL->GetComboBox()->RemoveEntries(0,TotalEntries-1);

    // Recreate the entries for each calibration point in the present
    // histogram calibration combo box
    stringstream SS;
    for(int i=0; i<=LastPointIndex; i++){
      SS << "Calibration point " << i;
      string Entry = SS.str();
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(Entry.c_str(), i);
      SS.str("");
    }

    // If the current histogram has calibration points then set the
    // combo box to diplay the to-be-set point
    if(LastPointIndex > 0){
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->Select(LastPointIndex);
      TI->SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }

    // ... otherwise display the no-calibration points values
    else{
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
      TI->SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0);
      TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(0);
    }
    break;
  }

  case SpectrumCalibrationPoint_CBL_ID:{
    
    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();    

    CalibrationDataStruct Cal = AAAcquisitionManager::GetInstance()->
      GetCalibrationDataStruct(Channel);
    
    int Size = Cal.PointID.size();
    
    if(SelectedID <= (Size-1)){
      
      double Energy = Cal.Energy.at(SelectedID);
      double PulseUnit = Cal.PulseUnit.at(SelectedID);
      
      TI->SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(Energy);
      TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(PulseUnit);

      double Value = Energy / TI->SpectrumMaxBin_NEL->GetEntry()->GetNumber();
      
      TI->DisplayHorizontalScale_THS->SetPointerPosition(Value);
    }
    else{
      TI->SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }
    break;
  }
  }
}


void AASubtabSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveID = ActiveEntry->WidgetId();

  TI->SaveSettings();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){
    
  case SpectrumCalibrationEnergy_NEL_ID:
  case SpectrumCalibrationPulseUnit_NEL_ID:{
    double Value = 0.;
    if(ActiveID == SpectrumCalibrationEnergy_NEL_ID)
      Value = TI->SpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();
    else
      Value = TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    
    // Normalize value for slider position setting from 0 to 1
    Value /= TI->SpectrumMaxBin_NEL->GetEntry()->GetNumber();
    
    TI->DisplayHorizontalScale_THS->SetPointerPosition(Value);
    break;
  }
  }
}


void AASubtabSlots::HandleTextButtons()
{
  TGTextButton *ActiveButton = (TGTextButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  AAAcquisitionManager *TheACQManager = AAAcquisitionManager::GetInstance();
  
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
      TheACQManager->SetAcquisitionTimeStart(time(NULL));

      // Get the stop time (i.e. amount of time to run in seconds)
      TheACQManager->SetAcquisitionTimeStop(TI->AQTime_NEL->GetEntry()->GetNumber());
      
      // Set the bool that will trigger the check of the timer against
      // the current time within the acquisition loop in RunDGScope()
      TheACQManager->SetAcquisitionTimerEnable(true);

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
    if(TheACQManager->GetAcquisitionEnable() and
       TheACQManager->GetAcquisitionTimerEnable())
      TheACQManager->StopAcquisition();
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

    // Get the data channel presently being histogrammed
    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();

    // Get the calibration point to be set
    uint SetPoint = TI->SpectrumCalibrationPoint_CBL->GetComboBox()->GetSelected();
    
    // Get the energy of the present calibration point
    double Energy = TI->SpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();
    
    // Get the pulse unit value of the present calibration point
    double PulseUnit = TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    
    TheACQManager->AddCalibrationPoint(Channel, SetPoint, Energy, PulseUnit);
    
    TI->UpdateAfterCalibrationPointAdded(SetPoint);
    
    break;
  }
    
  case SpectrumCalibrationCalibrate_TB_ID:{
    
    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();

    bool CalibrationEnable = TheACQManager->EnableCalibration(Channel);
    
    if(CalibrationEnable){
      TI->SpectrumCalibrationCalibrate_TB->SetText("Calibrated");
      TI->SpectrumCalibrationCalibrate_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
    }
    else
      {}
    
    break;
  }
    
  case SpectrumCalibrationPlot_TB_ID:{

    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    bool CalEnable = AAAcquisitionManager::GetInstance()->
      GetCalibrationEnable(Channel);
    
    if(CalEnable){
      //AAGraphicsManager::GetInstance()->PlotCalibration(Channel);
    }
    else{
    }

    break;
  }

  case SpectrumCalibrationReset_TB_ID:{

    // Get the current channel being histogrammed in DGScope
    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    bool CalibrationReset = TheACQManager->ResetCalibration(Channel);

    if(CalibrationReset){
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->RemoveAll();
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry("Calibration point 0", 0);
      TI->SpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
      TI->SpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.0);
      TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.0);

      TI->SpectrumCalibrationCalibrate_TB->SetText("Calibrate");
      TI->SpectrumCalibrationCalibrate_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
    }
    else
      {}
    
    break;
  }
    
  case SpectrumCalibrationLoad_TB_ID:{

    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};
    
    EFileDialogMode DialogType = kFDOpen;
    
    string FileName = TI->CreateFileDialog(FileTypes, DialogType);

    if(FileName == "NULL"){
    }
    else{
      
      int NumPoints = 0;
      bool CalibrationLoaded = TheACQManager->LoadCalibration(Channel, FileName, NumPoints);
      
      // Reset any preexisting calibrations
      TI->SpectrumCalibrationReset_TB->Clicked();
      
      if(CalibrationLoaded){
	for(int point=0; point<NumPoints; point++)
	  TI->UpdateAfterCalibrationPointAdded(point);
	
	TI->SpectrumCalibrationCalibrate_TB->Clicked();
      }
      else{
      }
    }
    break;
  }
    
  case SpectrumCalibrationWrite_TB_ID:{

    const char *FileTypes[] = {"ADAQ calibration file", "*.acal",
			       "All",                   "*.*",
			       0,0};

    EFileDialogMode DialogType = kFDSave;

    string FileName = TI->CreateFileDialog(FileTypes, DialogType);

    if(FileName == "NULL"){
    }
    else{
      int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
      
      TheACQManager->WriteCalibration(Channel, FileName);
    }
    break;
  }

    /*
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
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
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
    DGScopeDataStorageCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
    DGScopeDataStorageCreateFile_TB->SetText("Create ADAQ file");
    DGScopeDataStorageCloseFile_TB->SetState(kButtonDisabled);
    DGScopeDataStorageEnable_CB->SetState(kButtonUp);
    DGScopeDataStorageEnable_CB->SetState(kButtonDisabled);

    ROOTFileOpen = false;

    break;
  }

  case SpectrumFileName_TB_ID:{
    
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
      
      SpectrumFileName_TEL->GetEntry()->SetText(FileName_StripPath.c_str());
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

  TI->SaveSettings();
  
  switch(ActiveID){
    
  case SpectrumPulseHeight_RB_ID:
    
    if(TI->SpectrumPulseHeight_RB->IsDown()){
      TI->SpectrumLLD_NEL->GetEntry()->SetNumber(0);
      TI->SpectrumULD_NEL->GetEntry()->SetNumber(4095);
    }

    break;

  case SpectrumPulseArea_RB_ID:
    
    if(TI->SpectrumPulseArea_RB->IsDown()){
      TI->SpectrumLLD_NEL->GetEntry()->SetNumber(4000);
      TI->SpectrumULD_NEL->GetEntry()->SetNumber(100000);
    }

    break;
  }
}
