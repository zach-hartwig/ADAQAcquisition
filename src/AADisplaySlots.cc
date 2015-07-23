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

#include "ADAQDigitizer.hh"

#include "AADisplaySlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "AAGraphics.hh"


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
      
      // Special handling for acquisition timer 
      if(TheACQManager->GetAcquisitionTimerEnable())
	TheACQManager->SetAcquisitionTimerEnable(false);
    }
    
    // If acquisition is not presently running then start it
    else{
     
      // Update widget settings before turning acquisition on since
      // thread will remain in acquisition loop and not return immediately
      
      TI->SetAcquisitionWidgetState(false, kButtonDisabled);

      // Program the digitizers with the current settings
      bool DGProgramSuccess = TheVMEManager->ProgramDigitizers();

      bool DGChannelEnableSuccess = TheVMEManager->GetDGManager()->CheckForEnabledChannel();

      if(DGProgramSuccess and DGChannelEnableSuccess)
	TheACQManager->StartAcquisition();
      else
	TI->SetAcquisitionWidgetState(true, kButtonUp);
      break;
    }
    break;
  }
    
  case AQTrigger_TB_ID:{
    for(int i=0; i<30; i++)
      TheVMEManager->GetDGManager()->SendSWTrigger();
    break;
  }

  case DisplayUpdate_TB_ID:{
    
    if(TI->TheSettings->DisplayNonUpdateable)
      break;
    else{
      
      if(TI->TheSettings->WaveformMode and !TI->TheSettings->DisplayNonUpdateable){
	// Possibly implement future ability to update waveform manually
      }
      
      else if(TI->TheSettings->SpectrumMode and !TI->TheSettings->DisplayNonUpdateable){
	int Channel = TI->TheSettings->SpectrumChannel;
	TH1F *Spectrum_H = TheACQManager->GetSpectrum(Channel);
	AAGraphics::GetInstance()->PlotSpectrum(Spectrum_H);
      }
      else if(TI->TheSettings->PSDMode and !TI->TheSettings->DisplayNonUpdateable){
	int Channel = TI->TheSettings->PSDChannel;
	TH2F *PSDHistogram_H = TheACQManager->GetPSDHistogram(Channel);
	AAGraphics::GetInstance()->PlotPSDHistogram(PSDHistogram_H);
      }
      
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
{
  TI->SaveSettings();
  
  if(TI->TheSettings->SpectrumMode and 
     TI->TheSettings->SpectrumCalibrationEnable){
    
    double Max = TI->TheSettings->SpectrumMaxBin;
    double Pos = TI->DisplayHorizontalScale_THS->GetPointerPosition();
    
    TI->SpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(Max*Pos);
  }
}
