#include "ADAQDigitizer.hh"

#include "AADisplaySlots.hh"
#include "AAInterface.hh"

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

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;

  switch(ActiveID){
    
  case DGScopeStartStop_TB_ID:{
    
    // If DGScope acquisition is started
    if(ActiveButton->GetString()=="Stopped"){
      // Update button color from red to green and update text to "Stop"
      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(8));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      ActiveButton->SetText("Acquiring");

      // Begin acquisition
      TheVMEManager->SetDGAcquisitionEnable(true);

      // Update widget state for acquisition-on
      TI->SetAcquisitionWidgetState(true, kButtonUp);
      
      // Run the DGScope
      //RunDGScope();
    }

    // If DGScope acquisition is being stopped...
    else if(ActiveButton->GetString()=="Acquiring"){
      // Update button color from green to red and update text to "Start"
      ActiveButton->SetBackgroundColor(TI->ColorManager->Number2Pixel(2));
      ActiveButton->SetForegroundColor(TI->ColorManager->Number2Pixel(1));
      ActiveButton->SetText("Stopped");

      // Stop acquisition
      TheVMEManager->SetDGAcquisitionEnable(false);
      TheVMEManager->GetDGManager()->SWStopAcquisition();
      
      // Update widget state for acquisition-off
      TI->SetAcquisitionWidgetState(false, kButtonDisabled);

      // Determine if a ROOT file was open and receiving data; if so,
      // ensure that the data is written and the ROOT file is closed
      /*
      if(ROOTFileOpen){
	if(DGScopeDataStorageEnable_CB->IsDown())
	  DGScopeDataStorageEnable_CB->Clicked();
	
	if(OutputDataFile->IsOpen())
	  DGScopeDataStorageCloseFile_TB->Clicked();
      }
      */
    }
    break;
  }
    
  case DGScopeTrigger_TB_ID:{
    TheVMEManager->GetDGManager()->SendSWTrigger();
    break;
  }

  case DGScopeUpdatePlot_TB_ID:{
    
    int CurrentChannel = TI->DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
    
    if(TI->DGScopeUltraRate_RB->IsDown())
      break;
    else{
      //   if(TI->DGScopeSpectrum_H[CurrentChannel])
      //	{}//ForceSpectrumDrawing();
      break;
    }
  }
  }
}
