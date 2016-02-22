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

#ifndef __ADAQTypes_hh__
#define __ADAQTypes_hh__ 1

enum{
  //////////////////
  // Settings tab //
  //////////////////

  SetSettingsFileName_TB_ID,
  SaveSettingsToFile_TB_ID,
  LoadSettingsFromFile_TB_ID,
  SettingsFileName_TEL_ID,
  AutoSaveSettings_CB_ID,
  AutoLoadSettings_CB_ID,
  
  
  ////////////////////
  // Connection tab //
  ////////////////////

  VMEConnect_TB_ID,

  BRBoardEnable_TB_ID,
  BRBoardAddress_ID,

  HVBoardEnable_TB_ID,
  HVBoardAddress_ID,
  HVBoardLinkNumber_ID,

  DGBoardEnable_TB_ID,
  DGBoardAddress_ID,
  DGBoardLinkNumber_ID,

  DGStandardFW_RB_ID,
  DGPSDFW_RB_ID,

  
  ///////////////////
  // Registers tab //
  ///////////////////

  BRReadAddress_ID,
  BRReadValue_ID,
  BRRead_ID,
  BRWriteAddress_ID,
  BRWriteValue_ID,
  BRWrite_ID,

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
  DGCh8Enable_CB_ID,
  DGCh9Enable_CB_ID,
  DGCh10Enable_CB_ID,
  DGCh11Enable_CB_ID,
  DGCh12Enable_CB_ID,
  DGCh13Enable_CB_ID,
  DGCh14Enable_CB_ID,
  DGCh15Enable_CB_ID,

  DGCh0DCOffset_NEL_ID,
  DGCh1DCOffset_NEL_ID,
  DGCh2DCOffset_NEL_ID,
  DGCh3DCOffset_NEL_ID,
  DGCh4DCOffset_NEL_ID,
  DGCh5DCOffset_NEL_ID,
  DGCh6DCOffset_NEL_ID,
  DGCh7DCOffset_NEL_ID,
  DGCh8DCOffset_NEL_ID,
  DGCh9DCOffset_NEL_ID,
  DGCh10DCOffset_NEL_ID,
  DGCh11DCOffset_NEL_ID,
  DGCh12DCOffset_NEL_ID,
  DGCh13DCOffset_NEL_ID,
  DGCh14DCOffset_NEL_ID,
  DGCh15DCOffset_NEL_ID,
  
  DGCh0TriggerThreshold_NEL_ID,
  DGCh1TriggerThreshold_NEL_ID,
  DGCh2TriggerThreshold_NEL_ID,
  DGCh3TriggerThreshold_NEL_ID,
  DGCh4TriggerThreshold_NEL_ID,
  DGCh5TriggerThreshold_NEL_ID,
  DGCh6TriggerThreshold_NEL_ID,
  DGCh7TriggerThreshold_NEL_ID,
  DGCh8TriggerThreshold_NEL_ID,
  DGCh9TriggerThreshold_NEL_ID,
  DGCh10TriggerThreshold_NEL_ID,
  DGCh11TriggerThreshold_NEL_ID,
  DGCh12TriggerThreshold_NEL_ID,
  DGCh13TriggerThreshold_NEL_ID,
  DGCh14TriggerThreshold_NEL_ID,
  DGCh15TriggerThreshold_NEL_ID,

  DGCh0BaselineCalcMin_NEL_ID,
  DGCh1BaselineCalcMin_NEL_ID,
  DGCh2BaselineCalcMin_NEL_ID,
  DGCh3BaselineCalcMin_NEL_ID,
  DGCh4BaselineCalcMin_NEL_ID,
  DGCh5BaselineCalcMin_NEL_ID,
  DGCh6BaselineCalcMin_NEL_ID,
  DGCh7BaselineCalcMin_NEL_ID,
  DGCh8BaselineCalcMin_NEL_ID,
  DGCh9BaselineCalcMin_NEL_ID,
  DGCh10BaselineCalcMin_NEL_ID,
  DGCh11BaselineCalcMin_NEL_ID,
  DGCh12BaselineCalcMin_NEL_ID,
  DGCh13BaselineCalcMin_NEL_ID,
  DGCh14BaselineCalcMin_NEL_ID,
  DGCh15BaselineCalcMin_NEL_ID,

  DGCh0BaselineCalcMax_NEL_ID,
  DGCh1BaselineCalcMax_NEL_ID,
  DGCh2BaselineCalcMax_NEL_ID,
  DGCh3BaselineCalcMax_NEL_ID,
  DGCh4BaselineCalcMax_NEL_ID,
  DGCh5BaselineCalcMax_NEL_ID,
  DGCh6BaselineCalcMax_NEL_ID,
  DGCh7BaselineCalcMax_NEL_ID,
  DGCh8BaselineCalcMax_NEL_ID,
  DGCh9BaselineCalcMax_NEL_ID,
  DGCh10BaselineCalcMax_NEL_ID,
  DGCh11BaselineCalcMax_NEL_ID,
  DGCh12BaselineCalcMax_NEL_ID,
  DGCh13BaselineCalcMax_NEL_ID,
  DGCh14BaselineCalcMax_NEL_ID,
  DGCh15BaselineCalcMax_NEL_ID,
  
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
  AQPSDHistogram_RB_ID,
  
  DGTriggerType_CBL_ID,
  DGTriggerEdge_CBL_ID,
  DGPSDTriggerHoldoff_NEL_ID,
  DGTriggerCoincidenceEnable_CB_ID,
  DGTriggerCoincidenceLevel_CBL_ID,

  DGAcquisitionControl_CBL_ID,
  DGRecordLength_NEL_ID,
  DGPostTriggerSize_NEL_ID,
  DGPSDMode_CBL_ID,
  DGPSDListAnalysis_RB_ID,
  DGPSDWaveformAnalysis_RB_ID,
  AQTime_NEL_ID,
  AQTimer_NEFL_ID,
  AQTimerStart_TB_ID,
  AQTimerAbort_TB_ID,

  DGEventsBeforeReadout_NEL_ID,
  CheckBufferStatus_TB_ID,
  AQDataReductionEnable_CB_ID,
  AQDataReductionFactor_NEL_ID,
  DGZLEEnable_CB_ID,

  // Spectrum subtab

  SpectrumChannel_CBL_ID,
  SpectrumNumBins_NEL_ID,
  SpectrumMinBin_NEL_ID,
  SpectrumMaxBin_NEL_ID,

  SpectrumPulseHeight_RB_ID,
  SpectrumPulseArea_RB_ID,
  SpectrumLDEnable_CB_ID,
  SpectrumLLD_NEL_ID,
  SpectrumULD_NEL_ID,
  SpectrumLDTrigger_CB_ID,
  SpectrumLDTriggerChannel_CBL_ID,

  SpectrumCalibration_CB_ID,
  SpectrumCalibrationPoint_CBL_ID,
  SpectrumUseCalibrationSlider_CB_ID,
  SpectrumCalibrationEnergy_NEL_ID,
  SpectrumCalibrationPulseUnit_NEL_ID,
  SpectrumCalibrationUnit_CBL_ID,
  SpectrumCalibrationSetPoint_TB_ID,
  SpectrumCalibrationCalibrate_TB_ID,
  SpectrumCalibrationPlot_TB_ID,
  SpectrumCalibrationReset_TB_ID,
  SpectrumCalibrationLoad_TB_ID,
  SpectrumCalibrationWrite_TB_ID,

  // Pulse discrimination
  
  PSDChannel_CBL_ID,
  PSDYAxisTail_RB_ID,
  PSDYAxisTailTotal_RB_ID,

  // Persistent storage subtab

  WaveformFileName_TB_ID,
  WaveformCreateFile_TB_ID,
  WaveformCloseFile_TB_ID,
  WaveformCommentFile_TB_ID,
  WaveformStorageEnable_CB_ID,
  WaveformStoreRaw_CB_ID,
  WaveformStoreEnergyData_CB_ID,
  WaveformStorePSDData_CB_ID,
  
  WaveformOutput_RB_ID,
  SpectrumOutput_RB_ID,
  PSDHistogramOutput_RB_ID,
  ObjectOutputType_CBL_ID,
  ObjectOutputFileName_TB_ID,
  ObjectSaveWithTimeExtension_CB_ID,
  ObjectSave_TB_ID,
  
  CanvasFileName_TB_ID,
  CanvasSaveWithTimeExtension_CB_ID,
  CanvasSave_TB_ID,

  // Graphics subtab

  DisplayTrigger_CB_ID,
  DisplayBaselineBox_CB_ID,
  DisplayPSDLimits_CB_ID,
  DisplayZLEThreshold_CB_ID,
  DisplayLegend_CB_ID,
  DisplayGrid_CB_ID,
  DisplayXAxisLog_CB_ID,
  DisplayYAxisLog_CB_ID,

  DrawWaveformWithLine_RB_ID,
  DrawWaveformWithMarkers_RB_ID,
  DrawWaveformWithBoth_RB_ID,

  DrawSpectrumWithLine_RB_ID,
  DrawSpectrumWithMarkers_RB_ID,
  DrawSpectrumWithBars_RB_ID,

  DisplayTitlesEnable_CB_ID,
  DisplayTitle_TEL_ID,
  DisplayXTitle_TEL_ID,
  DisplayXSize_NEL_ID,
  DisplayXOffset_NEL_ID,
  DisplayYTitle_TEL_ID,
  DisplayYSize_NEL_ID,
  DisplayYOffset_NEL_ID,

  DisplayContinuous_RB_ID,
  DisplayUpdateable_RB_ID,
  DisplayNonUpdateable_RB_ID
};

struct CalibrationDataStruct{
  vector<int> PointID;
  vector<double> Energy;
  vector<double> PulseUnit;
};

#endif
