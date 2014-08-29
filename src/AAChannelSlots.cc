#include "ADAQDigitizer.hh"

#include "AAChannelSlots.hh"
#include "AAInterface.hh"

AAChannelSlots::AAChannelSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AAChannelSlots::~AAChannelSlots()
{;}


void AAChannelSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveID = ActiveEntry->WidgetId();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){
    
    // Set the channel trigger thresholds
  case DGScopeCh0TriggerThreshold_NEL_ID:
  case DGScopeCh1TriggerThreshold_NEL_ID:
  case DGScopeCh2TriggerThreshold_NEL_ID:
  case DGScopeCh3TriggerThreshold_NEL_ID:
  case DGScopeCh4TriggerThreshold_NEL_ID:
  case DGScopeCh5TriggerThreshold_NEL_ID:
  case DGScopeCh6TriggerThreshold_NEL_ID:
  case DGScopeCh7TriggerThreshold_NEL_ID:{

    // Only enable setting if digitizer is currently acquiring
    if(TheVMEManager->GetDGEnable()){
      
      // Get the channel number to be set
      uint32_t ch = TI->DGScopeChTriggerThreshold_NEL_ID_Map[ActiveID];
      
      // Get the trigger threshold value [ADC] to be set
      uint32_t thr = TI->DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber();
      
      // Set the channel trigger threshold 
      TheVMEManager->GetDGManager()->SetChannelTriggerThreshold(ch, thr);
    }
    break;
  }
  }
}
