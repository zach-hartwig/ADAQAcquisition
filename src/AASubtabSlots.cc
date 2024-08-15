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

#include <TGFileDialog.h>

#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "ADAQDigitizer.hh"

#include "AASubtabSlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "AAGraphics.hh"
#include "AAEditor.hh"

AASubtabSlots::AASubtabSlots(AAInterface *TheInterface)
  : TI(TheInterface),
    WaveformFileName("DefaultData.adaq.root"),
    ObjectFileName("DefaultObject.root"),
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
    if(ActiveButton->IsDown()){
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(true);
      TI->DGTriggerCoincidenceWindow_NEL->GetEntry()->SetState(true);
      TI->DGTriggerCoincidenceChannel1_CBL->GetComboBox()->SetEnabled(true);
      TI->DGTriggerCoincidenceChannel2_CBL->GetComboBox()->SetEnabled(true);
    }
    else{
      TI->DGTriggerCoincidenceLevel_CBL->GetComboBox()->SetEnabled(false);
      TI->DGTriggerCoincidenceWindow_NEL->GetEntry()->SetState(false);
      TI->DGTriggerCoincidenceChannel1_CBL->GetComboBox()->SetEnabled(false);
      TI->DGTriggerCoincidenceChannel2_CBL->GetComboBox()->SetEnabled(false);
    }
    break;

  case SpectrumCalibration_CB_ID:
    if(ActiveButton->IsDown())
      TI->SetCalibrationWidgetState(true, kButtonUp);
    else
      TI->SetCalibrationWidgetState(false, kButtonDisabled);
    break;
    
  case WaveformStorageEnable_CB_ID:
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
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();
  
  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;
  
  TI->SaveSettings();
  
  switch(ActiveID){
    
  case DGPSDMode_CBL_ID:
    
    switch(SelectedID){
      
    case 0: // PSD waveform only mode
      TI->DGPSDWaveformAnalysis_RB->SetState(kButtonDown);
      TI->DGPSDListAnalysis_RB->SetState(kButtonUp);
      TI->DGPSDListAnalysis_RB->SetState(kButtonDisabled);
      break;

    case 1: // PSD list only mode
      TI->DGPSDListAnalysis_RB->SetState(kButtonDown);
      TI->DGPSDWaveformAnalysis_RB->SetState(kButtonUp);
      TI->DGPSDWaveformAnalysis_RB->SetState(kButtonDisabled);
      break;
      
    case 2: // PSD list+waveform mode
      TI->DGPSDListAnalysis_RB->SetState(kButtonDown);
      TI->DGPSDWaveformAnalysis_RB->SetState(kButtonUp);
      break;
    }
    break;
    
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
  Int_t ActiveID = ActiveEntry->WidgetId();

  TI->SaveSettings();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){

  case DGTriggerCoincidenceWindow_NEL_ID:
    break;
    
  case SpectrumCalibrationEnergy_NEL_ID:
    break;

  case SpectrumCalibrationPulseUnit_NEL_ID:{
    Double_t Value = 0.;
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
      if(TheACQManager->GetADAQFileIsOpen() and
	 !TI->WaveformStorageEnable_CB->IsDown()){
	TI->WaveformStorageEnable_CB->SetState(kButtonDown);
	TI->SaveSettings();
      }
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
    
    TheVMEManager->GetDGManager()->GetChannelBufferStatus(BufferStatus);

    Double_t BufferLevel = 0.;
    if(TI->TheSettings->STDFirmware)
      TheVMEManager->GetDGManager()->GetSTDBufferLevel(BufferLevel);
    else if(TI->TheSettings->PSDFirmware)
      TheVMEManager->GetDGManager()->GetPSDBufferLevel(BufferLevel);
    
    TI->DGBufferStatus_PB->Reset();
    TI->DGBufferStatus_PB->Increment(BufferLevel*100);
    
    if(BufferLevel > 0.90){
      TI->DGBufferStatus_PB->SetBarColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOff));
      TI->DGBufferStatus_PB->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    }
    else{
      TI->DGBufferStatus_PB->SetBarColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
      TI->DGBufferStatus_PB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
    }
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
      TI->SpectrumCalibrationCalibrate_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    }
    else
      {}
    
    break;
  }
    
    
  case SpectrumCalibrationPlot_TB_ID:{

    int Channel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    bool CalEnable = TheACQManager->GetCalibrationEnable(Channel);
    
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
      TI->SpectrumCalibrationCalibrate_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
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
      
      // Reset any preexisting calibrations
      TI->SpectrumCalibrationReset_TB->Clicked();

      int NumPoints = 0;
      bool CalibrationLoaded = TheACQManager->LoadCalibration(Channel, FileName, NumPoints);
      
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
    
    const char *FileTypes[] = {"ADAQ ROOT file","*.adaq.root",
			       0, 0};
    
    WaveformFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(WaveformFileName == "NULL")
      WaveformFileName = "DefaultData.adaq.root";
    
    string FileNameNoPath = WaveformFileName;
    
    size_t Found = FileNameNoPath.find_last_of("/");
    if(Found != string::npos)
      FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
    
    TI->WaveformFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
    
    break;
  }

    
  case WaveformCreateFile_TB_ID:{
    
    // First call a "special" save of widget settings to account for
    // those widgets that can remain active during acquisition to
    // account for settings that may have changed since acquisition
    // started but before the ADAQ files has been created

    TI->SaveActiveSettings();

    // Second create the ADAQ file now that widget states have been
    // saved correctly
    
    TheACQManager->CreateADAQFile(WaveformFileName);

    
    // Finally set the widget states appropriately for the time that
    // data is being readout into the ADAQ file

    TI->AQTime_NEL->GetEntry()->SetState(false);
    TI->WaveformFileName_TB->SetState(kButtonDisabled);
    TI->WaveformCreateFile_TB->SetState(kButtonDisabled);
    TI->WaveformCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(TI->ButtonBackColorOn));
    TI->WaveformCreateFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(TI->ButtonForeColor));
    TI->WaveformCreateFile_TB->SetText("File open!");
    TI->WaveformCommentFile_TB->SetState(kButtonUp);
    TI->WaveformCloseFile_TB->SetState(kButtonUp);
    TI->WaveformStorageEnable_CB->SetState(kButtonUp);
    TI->WaveformStoreRaw_CB->SetState(kButtonDisabled);
    TI->WaveformStoreEnergyData_CB->SetState(kButtonDisabled);
    TI->WaveformStorePSDData_CB->SetState(kButtonDisabled);
    break;
  }

   
  case WaveformCommentFile_TB_ID:{
    // Create a new custome editor for creating ADAQ file comments
    AAEditor *Editor = new AAEditor(TI,500,500);

    // Load any previously written comments to the editor; this allows
    // the user to redo previously made comments just in case. In the
    // future, one could imagine loading a template comment file...
    Editor->LoadBuffer(TheACQManager->GetADAQFileComment());

    // Wait to execute subsequent code until the editor window has
    // been closed
    gClient->WaitFor(Editor->GetEditorWindow());

    // Get the text from the editor as a TString and pass it to the
    // acquisition manager. From there, it will be passed to the
    // ADAQReadoutManager and saved properly into the ADAQ file
    TheACQManager->SetADAQFileComment(Editor->GetEditorText());
    
    delete Editor;
    
    break;
  }
    
    
  case WaveformCloseFile_TB_ID:{

    // Do not let the use close/write the ADAQ file if data is still
    // be actively acquired. This forces the user to stop taking data
    // and then properly close/write the ADAQ file.
    
    if(TI->WaveformStorageEnable_CB->IsDown())
      break;
    
    TheACQManager->CloseADAQFile();
    
    if(!TheACQManager->GetADAQFileIsOpen()){
      TI->AQTime_NEL->GetEntry()->SetState(true);
      TI->WaveformFileName_TB->SetState(kButtonUp);
      TI->WaveformCreateFile_TB->SetState(kButtonUp);
      TI->WaveformCreateFile_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
      TI->WaveformCreateFile_TB->SetForegroundColor(TI->ColorManager->Number2Pixel(kBlack));
      TI->WaveformCreateFile_TB->SetText("Create");
      TI->WaveformCommentFile_TB->SetState(kButtonDisabled);
      TI->WaveformCloseFile_TB->SetState(kButtonDisabled);
      TI->WaveformStorageEnable_CB->SetState(kButtonUp);
      TI->WaveformStorageEnable_CB->SetState(kButtonDisabled);
      TI->WaveformStoreRaw_CB->SetState(kButtonUp);
      TI->WaveformStoreEnergyData_CB->SetState(kButtonUp);
      TI->WaveformStorePSDData_CB->SetState(kButtonUp);
    }
    
    break;
  }


  case ObjectOutputFileName_TB_ID:{
    
    // Set the allowable file type extensions. These will be used to
    // determine the format of the data output to file
    const char *FileTypes[] = {"ROOT File"             , "*.root",
			       "Space-separated format", "*.dat",
			       "Comma-separated format", "*.csv",
			       0, 0};
    
    ObjectFileName = TI->CreateFileDialog(FileTypes, kFDOpen);
    
    if(ObjectFileName == "NULL"){
    }
    else{
      string FileNameNoPath = ObjectFileName;
      
      size_t Found = FileNameNoPath.find_last_of("/");
      if(Found != string::npos)
	FileNameNoPath = FileNameNoPath.substr(Found+1, FileNameNoPath.size());
      
      TI->ObjectOutputFileName_TEL->GetEntry()->SetText(FileNameNoPath.c_str());
    }
    break;
  }
    
    
  case ObjectSave_TB_ID:{
    
    if(TI->ObjectSaveWithTimeExtension_CB->IsDown()){

      size_t Found0 = ObjectFileName.find_first_of(".");
      size_t Found1 = ObjectFileName.find_last_of(".");
      
      if(Found1 != string::npos){
	string FileName = ObjectFileName.substr(0, Found0);
	string FileExtension = ObjectFileName.substr(Found1, string::npos);
	
	time_t Time = time(NULL);
	
	stringstream SS;
	SS << "." << Time;
	string TimeString = SS.str();
	
	ObjectFileName = FileName + TimeString + FileExtension;
      }
    }
    
    if(TI->WaveformOutput_RB->IsDown())
      cout << "\nAASubtabSlots::HandleTextButtons() : Waveform output is not yet implemented!\n"                                                                              
	   << endl; 
    
    else if(TI->SpectrumOutput_RB->IsDown())
      AAAcquisitionManager::GetInstance()->SaveObjectData("Spectrum",
							  ObjectFileName);
    
    else if(TI->PSDHistogramOutput_RB->IsDown())
      AAAcquisitionManager::GetInstance()->SaveObjectData("PSDHistogram",
							  ObjectFileName);
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
    
    size_t Found0 = CanvasFileName.find_first_of(".");
    size_t Found1 = CanvasFileName.find_last_of(".");
    
    if(Found1 != string::npos){
      
      string FileName = CanvasFileName.substr(0, Found0);
      string FileExtension = CanvasFileName.substr(Found1, string::npos);
      
      if(TI->CanvasSaveWithTimeExtension_CB->IsDown()){
	time_t Time = time(NULL);
	
	stringstream SS;
	SS << "." << Time;
	string TimeString = SS.str();
	
	CanvasFileName = FileName + TimeString + FileExtension;
      }
      TI->DisplayCanvas_EC->GetCanvas()->Update();
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

  case AQWaveform_RB_ID:
  case AQSpectrum_RB_ID:
  case AQRate_RB_ID:
  case AQPSDHistogram_RB_ID:

    // Reset the horizontal and vertical sliders to restore the
    // default view when switching between display types
    TI->DisplayHorizontalScale_THS->SetPosition(0,1);
    TI->DisplayVerticalScale_DVS->SetPosition(0,1);
    break;
    
  case DGPSDListAnalysis_RB_ID:
    if(TI->DGPSDListAnalysis_RB->IsDown() and
       TI->DGPSDMode_CBL->GetComboBox()->GetSelected() != 1)
      TI->DGPSDWaveformAnalysis_RB->SetState(kButtonUp);
    break;
    
  case DGPSDWaveformAnalysis_RB_ID:
    if(TI->DGPSDWaveformAnalysis_RB->IsDown())
      TI->DGPSDListAnalysis_RB->SetState(kButtonUp);
    break;
    
  case SpectrumPulseHeight_RB_ID:
  case SpectrumPulseArea_RB_ID:
    
    TI->SpectrumLLD_NEL->GetEntry()->SetNumber(TI->TheSettings->SpectrumMinBin);
    TI->SpectrumULD_NEL->GetEntry()->SetNumber(TI->TheSettings->SpectrumMaxBin);
    break;

  case PSDYAxisTail_RB_ID:
    if(TI->PSDYAxisTail_RB->IsDown())
      TI->PSDYAxisTailTotal_RB->SetState(kButtonUp);
    break;
    
  case PSDYAxisTailTotal_RB_ID:
    if(TI->PSDYAxisTailTotal_RB->IsDown())
      TI->PSDYAxisTail_RB->SetState(kButtonUp);
    break;
    
  case DrawWaveformWithLine_RB_ID:
    TI->DrawWaveformWithMarkers_RB->SetState(kButtonUp);
    TI->DrawWaveformWithBoth_RB->SetState(kButtonUp);
    break;
    
  case DrawWaveformWithMarkers_RB_ID:
    TI->DrawWaveformWithLine_RB->SetState(kButtonUp);
    TI->DrawWaveformWithBoth_RB->SetState(kButtonUp);
    break;
    
  case DrawWaveformWithBoth_RB_ID:
    TI->DrawWaveformWithLine_RB->SetState(kButtonUp);
    TI->DrawWaveformWithMarkers_RB->SetState(kButtonUp);
    break;
    
  case DrawSpectrumWithLine_RB_ID:
    TI->DrawSpectrumWithMarkers_RB->SetState(kButtonUp);
    TI->DrawSpectrumWithBars_RB->SetState(kButtonUp);
    break;
    
  case DrawSpectrumWithMarkers_RB_ID:
    TI->DrawSpectrumWithLine_RB->SetState(kButtonUp);
    TI->DrawSpectrumWithBars_RB->SetState(kButtonUp);
    break;
    
  case DrawSpectrumWithBars_RB_ID:
    TI->DrawSpectrumWithLine_RB->SetState(kButtonUp);
    TI->DrawSpectrumWithMarkers_RB->SetState(kButtonUp);
    break;
  }
  TI->SaveSettings();
}
