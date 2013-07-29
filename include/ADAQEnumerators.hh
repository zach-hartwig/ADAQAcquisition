#ifndef __ADAQENUMERATORS_HH__
#define __ADAQENUMERATORS_HH__ 1

enum{
  // Widget IDs for the menu frame
  FileMenuExit_ID,

  // Widget IDs for the connection frame
  V1718Connect_TB_ID,
  
  V6534BoardEnable_TB_ID,
  V6534BoardAddress_ID,
  V6534ReadAddress_ID,
  V6534ReadValue_ID,
  V6534Read_ID,
  V6534WriteAddress_ID,
  V6534WriteValue_ID,
  V6534Write_ID,

  V1720BoardEnable_TB_ID,
  V1720BoardAddress_ID,
  V1720ReadAddress_ID,
  V1720ReadValue_ID,
  V1720Read_ID,
  V1720WriteAddress_ID,
  V1720WriteValue_ID,
  V1720Write_ID,

  // Widget IDs for the voltage frame
  HVChannel0Power_TB_ID,
  HVChannel1Power_TB_ID,
  HVChannel2Power_TB_ID,
  HVChannel3Power_TB_ID,
  HVChannel4Power_TB_ID,
  HVChannel5Power_TB_ID,
  
  HVEnableMonitoring_CB_ID,

  // Widget IDs for the scope frame
  DGScopeCh0Enable_CB_ID,
  DGScopeCh1Enable_CB_ID,
  DGScopeCh2Enable_CB_ID,
  DGScopeCh3Enable_CB_ID,
  DGScopeCh4Enable_CB_ID,
  DGScopeCh5Enable_CB_ID,
  DGScopeCh6Enable_CB_ID,
  DGScopeCh7Enable_CB_ID,

  DGScopeCh0DCOffset_NEL_ID,
  DGScopeCh1DCOffset_NEL_ID,
  DGScopeCh2DCOffset_NEL_ID,
  DGScopeCh3DCOffset_NEL_ID,
  DGScopeCh4DCOffset_NEL_ID,
  DGScopeCh5DCOffset_NEL_ID,
  DGScopeCh6DCOffset_NEL_ID,
  DGScopeCh7DCOffset_NEL_ID,

  DGScopeCh0TriggerThreshold_NEL_ID,
  DGScopeCh1TriggerThreshold_NEL_ID,
  DGScopeCh2TriggerThreshold_NEL_ID,
  DGScopeCh3TriggerThreshold_NEL_ID,
  DGScopeCh4TriggerThreshold_NEL_ID,
  DGScopeCh5TriggerThreshold_NEL_ID,
  DGScopeCh6TriggerThreshold_NEL_ID,
  DGScopeCh7TriggerThreshold_NEL_ID,

  DGScopeCh0BaselineCalcMin_NEL_ID,
  DGScopeCh1BaselineCalcMin_NEL_ID,
  DGScopeCh2BaselineCalcMin_NEL_ID,
  DGScopeCh3BaselineCalcMin_NEL_ID,
  DGScopeCh4BaselineCalcMin_NEL_ID,
  DGScopeCh5BaselineCalcMin_NEL_ID,
  DGScopeCh6BaselineCalcMin_NEL_ID,
  DGScopeCh7BaselineCalcMin_NEL_ID,

  DGScopeCh0BaselineCalcMax_NEL_ID,
  DGScopeCh1BaselineCalcMax_NEL_ID,
  DGScopeCh2BaselineCalcMax_NEL_ID,
  DGScopeCh3BaselineCalcMax_NEL_ID,
  DGScopeCh4BaselineCalcMax_NEL_ID,
  DGScopeCh5BaselineCalcMax_NEL_ID,
  DGScopeCh6BaselineCalcMax_NEL_ID,
  DGScopeCh7BaselineCalcMax_NEL_ID,

  DGScopeAcquisitionTimerStart_TB_ID,
  DGScopeSpectrumChannel_CBL_ID,
  DGScopeSpectrumAnalysisHeight_RB_ID,
  DGScopeSpectrumAnalysisArea_RB_ID,
  DGScopeSpectrumCalibrationPoint_CBL_ID,
  DGScopeSpectrumUseCalibrationSlider_CB_ID,
  DGScopeSpectrumFileName_TB_ID,
  DGScopeSaveSpectrum_TB_ID,
  DGScopeCanvasFileName_TB_ID,
  DGScopeSaveCanvas_TB_ID,

  DGScopeVerticalSlider_DVS_ID,
  DGScopeHorizontalSlider_THS_ID,

  DGScopeWaveform_RB_ID,
  DGScopeSpectrum_RB_ID,
  DGScopeHighRate_RB_ID,
  DGScopeUltraHighRate_RB_ID,

  DGScopeTriggerCoincidenceEnable_CB_ID,
  DGScopeChVerticalPosition_NEL_ID,

  DGScopeSpectrumCalibration_CB_ID,
  DGScopeSpectrumCalibrationEnergyPulseUnit_NEL_ID,
  DGScopeSpectrumCalibrationPulseUnit_NEL_ID,
  DGScopeSpectrumCalibrationSetPoint_TB_ID,
  DGScopeSpectrumCalibrationCalibrate_TB_ID,
  DGScopeSpectrumCalibrationPlot_TB_ID,
  DGScopeSpectrumCalibrationReset_TB_ID,
  
  DGScopeSpectrumBinNumber_NEL_ID,
  DGScopeSpectrumMinBin_NEL_ID,
  DGScopeSpectrumMaxBin_NEL_ID,
  DGScopeSpectrumXAxisLog_CB_ID,
  DGScopeSpectrumYAxisLog_CB_ID,
  DGScopeSpectrumAggregateRuns_CB_ID,

  DGScopeStartStop_TB_ID,
  DGScopeTrigger_TB_ID,
  DGScopeUpdatePlot_TB_ID,
  DGScopeSave_TB_ID,

  DGScopeTriggerMode_CBL_ID,

  DGScopeRecordLength_NEL_ID,
  DGScopePostTriggerSize_NEL_ID,

  DGScopeDataStorageCreateFile_TB_ID,
  DGScopeDataStorageCloseFile_TB_ID,

  DGScopeCheckBufferStatus_TB_ID
};

#endif
