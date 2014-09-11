#include <TGFileDialog.h>

#include <sstream>
#include <fstream>

#include "ADAQDigitizer.hh"

#include "AASubtabSlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "AAGraphics.hh"

AASubtabSlots::AASubtabSlots(AAInterface *TheInterface)
  : TI(TheInterface),
    WaveformFileName("DefaultWaveforms.adaq"),
    SpectrumFileName("DefaultSpectrum.root"),
    CanvasFileName("DefaultCanvas.eps")
{;}


AASubtabSlots::~AASubtabSlots()
{;}


void AASubtabSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();
  
  switch(ActiveID){

  case DGTriggerCoincidenceEnable_CB_ID:
    
    if(ActiveButton->IsDown())
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(true);
    else
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(false);

    break;


  case SpectrumCalibration_CB_ID:
    
    if(ActiveButton->IsDown())
      TI->SetCalibrationWidgetState(true, kButtonUp);
    else
      TI->SetCalibrationWidgetState(false, kButtonDisabled);
    
    break;
    

  case DisplayTitlesEnable_CB_ID:
    
    if(ActiveButton->IsDown())
      TI->SetTitlesWidgetState(true, kButtonUp);
    else
      TI->SetTitlesWidgetState(false, kButtonDisabled);
	 
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
    break;



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
      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
      ActiveButton->SetText("Waiting ...");
      
      // Get the start time (i.e. now)
      TheACQManager->SetAcquisitionTimeStart(time(NULL));
      
      // Get the stop time (i.e. amount of time to run in seconds)
      TheACQManager->SetAcquisitionTimeStop(TI->AQTime_NEL->GetEntry()->GetNumber());
      
      // Set the bool that will trigger the check of the timer against
      // the current time within the acquisition loop in RunDGScope()
      TheACQManager->SetAcquisitionTimerEnable(true);
      
      // Configure such that when an ADAQ file has been opened and the
      // AQ timer is running waveform data is being stored
      if(TheACQManager->GetADAQFileIsOpen())
	TI->WaveformStorageEnable_CB->SetState(kButtonDown);
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
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    }
    else
      {}
    
    break;
  }

    
  case SpectrumCalibrationPlot_TB_ID:{

    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    bool CalEnable = AAAcquisitionManager::GetInstance()->
      GetCalibrationEnable(Channel);
    
    if(CalEnable)
      AAGraphics::GetInstance()->PlotCalibration(Channel);
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
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
      TI->SpectrumCalibrationCalibrate_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(kBlack));
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

    string FileName = TI->CreateFileDialog(FileTypes, kFDSave);

    if(FileName == "NULL"){
    }
    else{
      int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
      
      TheACQManager->WriteCalibration(Channel, FileName);
    }
    break;
  }


  case WaveformFileName_TB_ID:{
    
    const char *FileTypes[] = {"ADAQ ROOT file","*.adaq",
			       "ADAQ ROOT file","*.root",
			       0, 0};

    WaveformFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(WaveformFileName == "NULL"){
    }
    else{
      string FileNameNoPath = WaveformFileName;
      
      size_t Found = FileNameNoPath.find_last_of("/");
      if(Found != string::npos)
	FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
      
      TI->WaveformFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
    }
    
    break;
  }

    
  case WaveformCreateFile_TB_ID:{
    
    AAAcquisitionManager::GetInstance()->CreateADAQFile(WaveformFileName);
    
    // Set widget states appropriately

    TI->WaveformCreateFile_TB->SetState(kButtonDisabled);
    TI->WaveformCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
    TI->WaveformCreateFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    TI->WaveformCreateFile_TB->SetText("ADAQ file created");
    TI->WaveformCloseFile_TB->SetState(kButtonUp);
    TI->WaveformStorageEnable_CB->SetState(kButtonUp);
  
    break;
  }


  case WaveformCloseFile_TB_ID:{

    AAAcquisitionManager::GetInstance()->CloseADAQFile();
    
    // Set widget states appropriately.

    TI->WaveformCreateFile_TB->SetState(kButtonUp);
    TI->WaveformCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
    TI->WaveformCreateFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
    TI->WaveformCreateFile_TB->SetText("Create ADAQ file");
    TI->WaveformCloseFile_TB->SetState(kButtonDisabled);
    TI->WaveformStorageEnable_CB->SetState(kButtonUp);
    TI->WaveformStorageEnable_CB->SetState(kButtonDisabled);
    
    break;
  }


  case SpectrumFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"Space-separated format", "*.dat",
			       "Comma-separated format", "*.csv",
			       "ROOT File"             , "*.root",
			       0, 0};
    
    SpectrumFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(SpectrumFileName == "NULL"){
    }
    else{
      string FileNameNoPath = SpectrumFileName;
      
      size_t Found = FileNameNoPath.find_last_of("/");
      if(Found != string::npos)
	FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
      
      TI->SpectrumFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
    }
    break;
  }

    
  case SpectrumSave_TB_ID:{
    
    AAAcquisitionManager::GetInstance()->SaveSpectrum(SpectrumFileName);

    break;
  }
    
    
  case CanvasFileName_TB_ID:{
    
    const char *FileTypes[] = {"EPS file", "*.eps",
			       "PS file",  "*.ps",
			       "PDF file", "*.pdf",
			       "PNG file", "*.png",
			       "JPEG file", "*.jpeg",
			       0, 0};

    CanvasFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(CanvasFileName == "NULL"){
    }
    else{
      string FileNameNoPath = CanvasFileName;
      
      size_t Found = FileNameNoPath.find_last_of("/");
      if(Found != string::npos)
	FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
      
      TI->CanvasFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
    }

    break;
  }

    
  case CanvasSave_TB_ID:{

    size_t Found = CanvasFileName.find_last_of(".");

    if(Found != string::npos){

      string FileExtension = CanvasFileName.substr(Found, string::npos);
      
      if(TI->CanvasSaveWithTimeExtension_CB->IsDown()){
	time_t Time = time(NULL);
	
	stringstream SS;
	SS << "." << Time;
	string TimeString = SS.str();
	
	CanvasFileName.insert(Found, TimeString);
      }
      
      TI->DisplayCanvas_EC->GetCanvas()->Print(CanvasFileName.c_str(),
					       FileExtension.substr(1).c_str());
    }
    break;
  }
  }
}

  

void AASubtabSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();
  
  switch(ActiveID){
    
  case SpectrumPulseHeight_RB_ID:
  case SpectrumPulseArea_RB_ID:
    
    TI->SpectrumLLD_NEL->GetEntry()->SetNumber(TI->TheSettings->SpectrumMinBin);
    TI->SpectrumULD_NEL->GetEntry()->SetNumber(TI->TheSettings->SpectrumMaxBin);
    break;
  }
}
