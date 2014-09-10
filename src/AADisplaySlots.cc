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
      
      // Stop data acquisition first
      TheACQManager->StopAcquisition();
      
      // Update widgets for acquisition-off settings
      TI->SetAcquisitionWidgetState(true, kButtonUp);

      // Special handling for acquistion timer 
      if(TheACQManager->GetAcquisitionTimerEnable())
	TheACQManager->SetAcquisitionTimerEnable(false);
    }
    
    // If acquisition is not presently running then start it
    else{

      // Update widget settings before turning acquisition on since
      // thread will remain in acquisition loop and not return immediately

      TI->SetAcquisitionWidgetState(false, kButtonDisabled);

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


void AADisplaySlots::HandleDoubleSliders()
{
  TI->SaveSettings();
  
  TGDoubleSlider *ActiveSlider = (TGDoubleSlider *) gTQSender;
  int ActiveID = ActiveSlider->WidgetId();
  
  switch(ActiveID){

  case DisplayHorizontalScale_THS_ID:
    break;

  case DisplayVerticalScale_DVS_ID:
    break;
  }
}


void AADisplaySlots::HandleSliderPointers()
{}
