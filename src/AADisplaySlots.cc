#include "ADAQDigitizer.hh"

#include "AADisplaySlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"


AADisplaySlots::AADisplaySlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AADisplaySlots::~AADisplaySlots()
{;}


void AADisplaySlots::HandleTextButtons()
{
  // Get pointers and the widget ID for the active (ie, clicked) text button
  TGTextButton *ActiveButton = (TGTextButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  AAAcquisitionManager *TheACQManager = AAAcquisitionManager::GetInstance();
  
  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;
  
  switch(ActiveID){
    
  case AQStartStop_TB_ID:{

    // If acquisition is presently running ...
    if(TheACQManager->GetAcquisitionEnable()){

      // Stop data acquisition 
      TheACQManager->StopAcquisition();

      // Determine if a ROOT file was open and receiving data; if so,
      // ensure that the data is written and the ROOT file is closed
      /*
      if(ROOTFileOpen){
	if(DGScopeDataStorageEnable_CB->IsDown())
	  DGScopeDataStorageEnable_CB->Clicked();
	
	if(OutputDataFile->IsOpen())
	  DGScopeDataStorageCloseFile_TB->Clicked();
      */

      // Update widgets for acquisition-off settings

      TI->SetAcquisitionWidgetState(true, kButtonUp);

      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      ActiveButton->SetText("Stopped");

      if(TheACQManager->GetAcquisitionTimerEnable()){
	TI->AQTimerStart_TB->SetBackgroundColor(TI->ColorManager->Number2Pixel(18));
	TI->AQTimerStart_TB->SetText("Start timer");
	
	TheACQManager->SetAcquisitionTimerEnable(false);
      }

      TI->WaveformCreateFile_TB->SetState(kButtonDisabled);
      TI->WaveformCloseFile_TB->SetState(kButtonDisabled);
      TI->WaveformEnable_CB->SetState(kButtonUp);
      TI->WaveformEnable_CB->SetState(kButtonDisabled);
      
    }

    // If acquisition is not presently running then start it
    else{

      // Update widgets for acquisition-on settings

      TI->SetAcquisitionWidgetState(false, kButtonDisabled);

      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      ActiveButton->SetText("Acquiring");
      
      // Program the digitizers with the current settings
      TheVMEManager->ProgramDigitizers();
      
      // Start data acquisition
      TheACQManager->StartAcquisition();
    }
    break;
  }
    
  case AQTrigger_TB_ID:{
    TheVMEManager->GetDGManager()->SendSWTrigger();
    break;
  }

  case DisplayUpdate_TB_ID:{
    
    int CurrentChannel = TI->SpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(TI->AQUltraRate_RB->IsDown())
      break;
    else{
      //   if(TI->DGScopeSpectrum_H[CurrentChannel])
      //	{}//ForceSpectrumDrawing();
      break;
    }
  }
  }
}
