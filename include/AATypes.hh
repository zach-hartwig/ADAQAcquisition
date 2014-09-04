#ifndef __ADAQTypes_hh__
#define __ADAQTypes_hh__ 1

enum{
  ////////////////////
  // Connection tab //
  ////////////////////

  VMEConnect_TB_ID,

  V1718BoardEnable_TB_ID,
  V1718BoardAddress_ID,

  HVBoardEnable_TB_ID,
  HVBoardAddress_ID,

  DGBoardEnable_TB_ID,
  DGBoardAddress_ID,

  
  ///////////////////
  // Registers tab //
  ///////////////////

  V1718ReadAddress_ID,
  V1718ReadValue_ID,
  V1718Read_ID,
  V1718WriteAddress_ID,
  V1718WriteValue_ID,
  V1718Write_ID,
    

  HVReadAddress_ID,
  HVReadValue_ID,
  HVRead_ID,
  HVWriteAddress_ID,
  HVWriteValue_ID,
  HVWrite_ID,


  DGReadAddress_ID,
  DGReadValue_ID,
  DGRead_ID,
  DGWriteAddress_ID,
  DGWriteValue_ID,
  DGWrite_ID,

  ////////////////
  // Pulser tab //
  ////////////////

  V1718PulserA_TB_ID, 
  V1718PulserB_TB_ID,


  ///////////////////
  // Voltage frame //
  ///////////////////

  HVCh0Power_TB_ID,
  HVCh1Power_TB_ID,
  HVCh2Power_TB_ID,
  HVCh3Power_TB_ID,
  HVCh4Power_TB_ID,
  HVCh5Power_TB_ID,
  
  HVEnableMonitoring_CB_ID,


  ///////////////////////
  // Acquisition frame //
  ///////////////////////

  ////////////////////////////////////////////////////////////////////////
  // Widget IDs for the channel subframe (right side of acquisition frame)

  DGCh0Enable_CB_ID,
  DGCh1Enable_CB_ID,
  DGCh2Enable_CB_ID,
  DGCh3Enable_CB_ID,
  DGCh4Enable_CB_ID,
  DGCh5Enable_CB_ID,
  DGCh6Enable_CB_ID,
  DGCh7Enable_CB_ID,

  DGCh0DCOffset_NEL_ID,
  DGCh1DCOffset_NEL_ID,
  DGCh2DCOffset_NEL_ID,
  DGCh3DCOffset_NEL_ID,
  DGCh4DCOffset_NEL_ID,
  DGCh5DCOffset_NEL_ID,
  DGCh6DCOffset_NEL_ID,
  DGCh7DCOffset_NEL_ID,

  DGCh0TriggerThreshold_NEL_ID,
  DGCh1TriggerThreshold_NEL_ID,
  DGCh2TriggerThreshold_NEL_ID,
  DGCh3TriggerThreshold_NEL_ID,
  DGCh4TriggerThreshold_NEL_ID,
  DGCh5TriggerThreshold_NEL_ID,
  DGCh6TriggerThreshold_NEL_ID,
  DGCh7TriggerThreshold_NEL_ID,

  DGCh0BaselineCalcMin_NEL_ID,
  DGCh1BaselineCalcMin_NEL_ID,
  DGCh2BaselineCalcMin_NEL_ID,
  DGCh3BaselineCalcMin_NEL_ID,
  DGCh4BaselineCalcMin_NEL_ID,
  DGCh5BaselineCalcMin_NEL_ID,
  DGCh6BaselineCalcMin_NEL_ID,
  DGCh7BaselineCalcMin_NEL_ID,

  DGCh0BaselineCalcMax_NEL_ID,
  DGCh1BaselineCalcMax_NEL_ID,
  DGCh2BaselineCalcMax_NEL_ID,
  DGCh3BaselineCalcMax_NEL_ID,
  DGCh4BaselineCalcMax_NEL_ID,
  DGCh5BaselineCalcMax_NEL_ID,
  DGCh6BaselineCalcMax_NEL_ID,
  DGCh7BaselineCalcMax_NEL_ID,


  ///////////////////////////////////////////////////////////////////////
  // Widget IDs for the display subframe (top-right of acquisition frame)
  
  DisplayVerticalScale_DVS_ID,
  DisplayHorizontalScale_THS_ID,
  AQStartStop_TB_ID,
  AQTrigger_TB_ID,
  DisplayUpdate_TB_ID,
  

  /////////////////////////////////////////////////////////////////////////////
  // Widget IDs for the acquisition subtabs (bottom-right of acquisition frame)

  // Acquisition subtab

  AQWaveform_RB_ID,
  AQSpectrum_RB_ID,
  AQHighRate_RB_ID,
  AQUltraRate_RB_ID,

  DGTriggerType_CBL_ID,
  DGTriggerEdge_CBL_ID,
  DGTriggerCoincidenceEnable_CB_ID,
  DGTriggerCoincidenceLevel_CBL_ID,

  DGRecordLength_NEL_ID,
  DGPostTriggerSize_NEL_ID,
  AQTime_NEL_ID,
  AQTimer_NEFL_ID,
  AQTimerStart_TB_ID,
  AQTimerAbort_TB_ID,

  DGEventsBeforeReadout_NEL_ID,
  CheckBufferStatus_TB_ID,
  AQDataReductionEnable_CB_ID,
  AQDataReductionFactor_NEL_ID,
  DGZSEnable_CB_ID,

  // Spectrum subtab

  SpectrumChannel_CBL_ID,
  SpectrumNumBins_NEL_ID,
  SpectrumMinBin_NEL_ID,
  SpectrumMaxBin_NEL_ID,

  SpectrumPulseHeight_RB_ID,
  SpectrumPulseArea_RB_ID,
  SpectrumLLD_NEL_ID,
  SpectrumULD_NEL_ID,
  SpectrumLDTrigger_CB_ID,
  SpectrumLDTriggerChannel_CBL_ID,

  SpectrumCalibration_CB_ID,
  SpectrumCalibrationPoint_CBL_ID,
  SpectrumUseCalibrationSlider_CB_ID,
  SpectrumCalibrationEnergy_NEL_ID,
  SpectrumCalibrationPulseUnit_NEL_ID,
  SpectrumCalibrationSetPoint_TB_ID,
  SpectrumCalibrationCalibrate_TB_ID,
  SpectrumCalibrationPlot_TB_ID,
  SpectrumCalibrationReset_TB_ID,
  SpectrumCalibrationLoad_TB_ID,
  SpectrumCalibrationWrite_TB_ID,

  // Graphics subtab

  DisplayXAxisLog_CB_ID,
  DisplayYAxisLog_CB_ID,

  // Persistent storage subtab

  WaveformFileName_TB_ID,
  WaveformCreateFile_TB_ID,
  WaveformCloseFile_TB_ID,
  WaveformEnable_CB_ID,

  SpectrumFileName_TB_ID,
  SpectrumSaveWithTimeExtension_CB_ID,
  SpectrumSave_TB_ID,
  
  CanvasFileName_TB_ID,
  CanvasSaveWithTimeExtension_CB_ID,
  CanvasSave_TB_ID
};

struct CalibrationDataStruct{
  vector<int> PointID;
  vector<double> Energy;
  vector<double> PulseUnit;
};

#endif
