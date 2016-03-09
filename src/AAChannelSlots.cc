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

#include "AAChannelSlots.hh"
#include "AAInterface.hh"
#include "AAVMEManager.hh"

AAChannelSlots::AAChannelSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AAChannelSlots::~AAChannelSlots()
{;}


void AAChannelSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  TI->SaveSettings();

  switch(ActiveID){
    
  case DGChannelLockToZero_CB_ID:
    if(ActiveButton->IsDown())
      TI->UpdateChannelSettingsToChannelZero();
    break;
    
  default:
    break;
  }
}


void AAChannelSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveID = ActiveEntry->WidgetId();
  
  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){
    
    // Set the channel trigger thresholds
  case DGCh0TriggerThreshold_NEL_ID:
  case DGCh1TriggerThreshold_NEL_ID:
  case DGCh2TriggerThreshold_NEL_ID:
  case DGCh3TriggerThreshold_NEL_ID:
  case DGCh4TriggerThreshold_NEL_ID:
  case DGCh5TriggerThreshold_NEL_ID:
  case DGCh6TriggerThreshold_NEL_ID:
  case DGCh7TriggerThreshold_NEL_ID:{

    // Only enable setting if digitizer is currently acquiring
    if(TheVMEManager->GetDGEnable()){
      
      // Get the channel number to be set
      uint32_t ch = TI->DGChTriggerThreshold_NEL_ID_Map[ActiveID];
      
      // Get the trigger threshold value [ADC] to be set
      uint32_t thr = TI->DGChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
      
      // Set the channel trigger threshold 
      TheVMEManager->GetDGManager()->SetChannelTriggerThreshold(ch, thr);
    }
    break;
  }
  }
}


void AAChannelSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  TI->SaveSettings();
}
