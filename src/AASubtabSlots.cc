#include "ADAQDigitizer.hh"

#include "AASubtabSlots.hh"
#include "AAInterface.hh"

AASubtabSlots::AASubtabSlots(AAInterface *TheInterface)
  : TI(TheInterface)
{;}


AASubtabSlots::~AASubtabSlots()
{;}


void AASubtabSlots::HandleCheckButtons()
{
  TGCheckButton *ActiveButton = (TGCheckButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  switch(ActiveID){

    // Enable the calibration widgets
  case DGScopeSpectrumCalibration_CB_ID:

    if(TI->DGScopeSpectrumCalibration_CB->IsDown())
      TI->SetCalibrationWidgetState(true, kButtonUp);
    else
      TI->SetCalibrationWidgetState(false, kButtonDisabled);

    break;
  }
}


void AASubtabSlots::HandleComboBoxes(int ActivexID, int SelectedID)
{
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  if(!TheVMEManager->GetVMEConnectionEstablished())
    return;

  /*
  switch(ActiveID){
    
  case DGScopeSpectrumChannel_CBL_ID:{

    // In order to ensure that the calibration set point combo box
    // displays the correct number of calibration points, we need to
    // rebuild the combo box entry list each time the user selects a
    // new histogram channel to plot using this combo box

    // Get the number of calibration points for the current histogram
    // (SelectedID == histogram channel for this combo box)
    int LastPointIndex = CalibrationData[SelectedID].PointID.size();

    // Remove the previous entries from the combo box
    int TotalEntries = DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->GetNumberOfEntries();
    DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->RemoveEntries(0,TotalEntries-1);

    // Recreate the entries for each calibration point in the present
    // histogram calibration combo box
    stringstream ss;
    for(int i=0; i<=LastPointIndex; i++){
      ss << "Calibration point " << i;
      string Entry = ss.str();
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->AddEntry(Entry.c_str(), i);
      ss.str("");
    }

    // If the current histogram has calibration points then set the
    // combo box to diplay the to-be-set point
    if(LastPointIndex > 0){
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(LastPointIndex);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }

    // ... otherwise display the no-calibration points values
    else{
      DGScopeSpectrumCalibrationPoint_CBL->GetComboBox()->Select(0);
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(0);
    }
    break;
  }

  case DGScopeSpectrumCalibrationPoint_CBL_ID:{

    int SpectrumChannel = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();    
    int Size = CalibrationData[SpectrumChannel].PointID.size();

    if(SelectedID <= (Size-1)){

      double Energy = CalibrationData[SpectrumChannel].Energy.at(SelectedID);
      double PulseUnit = CalibrationData[SpectrumChannel].PulseUnit.at(SelectedID);

      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(Energy);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(PulseUnit);

      double Value = Energy/DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();

      DGScopeHorizontalScale_THS->SetPointerPosition(Value);
    }
    else{
      DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->SetNumber(0.);
      DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(1.);
    }
    break;
  }
  }
  */
}


void AASubtabSlots::HandleNumberEntries()
{
  // Get the pointer and the widget ID for the active number entry
  TGNumberEntry *ActiveEntry = (TGNumberEntry *) gTQSender;
  int ActiveID = ActiveEntry->WidgetId();
  
  AAVMEManager *TheVMEManager = AAVMEManager::GetInstance();

  switch(ActiveID){
    
  case DGScopeSpectrumCalibrationEnergy_NEL_ID:
  case DGScopeSpectrumCalibrationPulseUnit_NEL_ID:{
    double Value = 0.;
    if(ActiveID == DGScopeSpectrumCalibrationEnergy_NEL_ID)
      Value = TI->DGScopeSpectrumCalibrationEnergy_NEL->GetEntry()->GetNumber();
    else
      Value = TI->DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
    
    // Normalize value for slider position setting from 0 to 1
    Value /= TI->DGScopeSpectrumMaxBin_NEL->GetEntry()->GetNumber();
    
    TI->DGScopeHorizontalScale_THS->SetPointerPosition(Value);
    break;
  }
  }
}


void AASubtabSlots::HandleTextButtons()
{


}


void AASubtabSlots::HandleRadioButtons()
{
  TGRadioButton *ActiveButton = (TGRadioButton *) gTQSender;
  int ActiveID = ActiveButton->WidgetId();
  
  switch(ActiveID){
    
  case DGScopeSpectrumAnalysisHeight_RB_ID:
    
    if(TI->DGScopeSpectrumAnalysisHeight_RB->IsDown()){
      TI->DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(0);
      TI->DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(4095);
    }

    break;

  case DGScopeSpectrumAnalysisArea_RB_ID:
    
    if(TI->DGScopeSpectrumAnalysisArea_RB->IsDown()){
      TI->DGScopeSpectrumAnalysisLLD_NEL->GetEntry()->SetNumber(4000);
      TI->DGScopeSpectrumAnalysisULD_NEL->GetEntry()->SetNumber(100000);
    }

    break;
  }
}
