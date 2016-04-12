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
    if(ActiveButton->IsDown()){
      TI->UpdateChannelSettingsToChannelZero();
      TI->SaveSettings();
    }
    break;
    
  default:
    break;
  }
}


void AAChannelSlots::HandleComboBoxes(Int_t ActiveID, Int_t SelectedID)
{
  TI->SaveSettings();
}

void AAChannelSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  Int_t ActiveID = ActiveEntry->WidgetId();

  TI->SaveSettings();

  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  // x720 + DPP-PSD specific settings

  ZBoardType DGType = TheVMEManager->GetDGManager()->GetBoardType();
  if(DGType == zDT5790M or DGType == zDT5790N or DGType == zDT5790P){
    
    if(ActiveID >= DGCh0RecordLength_NEL_ID and ActiveID <= DGCh15RecordLength_NEL_ID){
      if(ActiveID == DGCh0RecordLength_NEL_ID)
	TI->DGChRecordLength_NEL[1]->GetEntry()->SetIntNumber(ActiveEntry->GetIntNumber());
      
      else if(ActiveID == DGCh1RecordLength_NEL_ID)
	TI->DGChRecordLength_NEL[0]->GetEntry()->SetIntNumber(ActiveEntry->GetIntNumber());
    }
    
    if(ActiveID >= DGCh0PreTrigger_NEL_ID and ActiveID <= DGCh15PreTrigger_NEL_ID){
      if(ActiveID == DGCh0PreTrigger_NEL_ID)
	TI->DGChPreTrigger_NEL[1]->GetEntry()->SetIntNumber(ActiveEntry->GetIntNumber());
      
      else if(ActiveID == DGCh1PreTrigger_NEL_ID)
	TI->DGChPreTrigger_NEL[0]->GetEntry()->SetIntNumber(ActiveEntry->GetIntNumber());
    }
  }
}


void AAChannelSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  Int_t ActiveID = ActiveButton->WidgetId();

  TI->SaveSettings();
}
