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

#ifndef __AASettings_hh__
#define __AASettings_hh__ 1

#include <TObject.h>

#include <vector>
#include <iostream>
using namespace std;

class AASettings : public TObject
{
public:
  
  AASettings(int Channels){
    ChEnable.resize(Channels);
    ChPosPolarity.resize(Channels);
    ChNegPolarity.resize(Channels);
    ChDCOffset.resize(Channels);
    ChTriggerThreshold.resize(Channels);

    // CAEN Standard firmware specific settings
    
    ChZLEThreshold.resize(Channels);
    ChZLEForward.resize(Channels);
    ChZLEBackward.resize(Channels);
    ChZLEPosLogic.resize(Channels);
    ChZLENegLogic.resize(Channels);
    ChBaselineCalcMin.resize(Channels);
    ChBaselineCalcMax.resize(Channels);
    ChPSDTotalStart.resize(Channels);
    ChPSDTotalStop.resize(Channels);
    ChPSDTailStart.resize(Channels);
    ChPSDTailStop.resize(Channels);

    // CAEN DPP-PSD firwmare specific settings
    ChRecordLength.resize(Channels);
    ChBaselineSamples.resize(Channels);
    ChChargeSensitivity.resize(Channels);
    ChPSDCut.resize(Channels);
    ChTriggerConfig.resize(Channels);
    ChTriggerValidation.resize(Channels);
    ChShortGate.resize(Channels);
    ChLongGate.resize(Channels);
    ChPreTrigger.resize(Channels);
    ChGateOffset.resize(Channels);
  }

  /////////////////////////////////
  // VME connection widget settings

  Bool_t STDFirmware, PSDFirmware;
  
  //////////////////////////
  // Channel widget settings
  
  vector<Bool_t>  ChEnable;
  vector<Bool_t>  ChPosPolarity;
  vector<Bool_t>  ChNegPolarity;
  vector<Int_t>   ChDCOffset;
  vector<Int_t>   ChTriggerThreshold;

  // CAEN Standard firmware specific settings

  vector<Int_t>   ChZLEThreshold;
  vector<Int_t>   ChZLEForward;
  vector<Int_t>   ChZLEBackward;
  vector<Bool_t>  ChZLEPosLogic;
  vector<Bool_t>  ChZLENegLogic;
  vector<Int_t>   ChBaselineCalcMin;
  vector<Int_t>   ChBaselineCalcMax;
  vector<Int_t>   ChPSDTotalStart;
  vector<Int_t>   ChPSDTotalStop;
  vector<Int_t>   ChPSDTailStart;
  vector<Int_t>   ChPSDTailStop;

  // CAEN DPP-PSD firmware specific settings

  vector<Int_t> ChRecordLength;
  vector<Int_t> ChBaselineSamples;
  vector<Int_t> ChChargeSensitivity;
  vector<Int_t> ChPSDCut;
  vector<Int_t> ChTriggerConfig;
  vector<Int_t> ChTriggerValidation;
  vector<Int_t> ChShortGate;
  vector<Int_t> ChLongGate;
  vector<Int_t> ChPreTrigger;
  vector<Int_t> ChGateOffset;
  

  //////////////////////////
  // Display widget settings

  Double_t HorizontalSliderPtr;
  Double_t HorizontalSliderMin, HorizontalSliderMax;
  Double_t VerticalSliderMin, VerticalSliderMax;


  ///////////////////////////////////////
  // Acquisition control widget settings

  // Type
  Bool_t WaveformMode;
  Bool_t SpectrumMode;
  Bool_t PSDMode;
  Bool_t PSDListAnalysis, PSDWaveformAnalysis;
  
  // Trigger
  Int_t TriggerType, TriggerEdge;
  string TriggerTypeName, TriggerEdgeName;
  Int_t PSDTriggerHoldoff;
  Bool_t TriggerCoincidenceEnable;
  Int_t TriggerCoincidenceLevel;

  
  // Acquisition
  Int_t AcquisitionControl;
  string AcquisitionControlName;
  Int_t RecordLength;
  Int_t PostTrigger;
  Int_t PSDOperationMode;
  Int_t AcquisitionTime;

  // Readout
  Int_t EventsBeforeReadout;
  Bool_t DataReductionEnable;
  Int_t DataReductionFactor;
  Bool_t ZeroSuppressionEnable;

  
  ////////////////////////////////////
  // Spectrum creation widget settings

  // Histogram
  Int_t SpectrumChannel;
  Int_t SpectrumNumBins;
  Double_t SpectrumMinBin;
  Double_t SpectrumMaxBin;
  
  // Analysis
  Bool_t SpectrumPulseHeight, SpectrumPulseArea;
  Int_t SpectrumLLD, SpectrumULD;
  Bool_t LDEnable;
  Bool_t LDTrigger;
  Int_t LDChannel;

  // Calibration
  Bool_t SpectrumCalibrationEnable;
  Bool_t SpectrumCalibrationUseSlider;
  string SpectrumCalibrationUnit;


  //////////////////////////////
  // Pulse discrimination limits

  Int_t PSDChannel;
  Bool_t PSDYAxisTail, PSDYAxisTailTotal;
  Double_t PSDThreshold;
  Int_t PSDTotalBins, PSDTailBins;
  Double_t PSDTotalMinBin, PSDTotalMaxBin;
  Double_t PSDTailMinBin, PSDTailMaxBin;
  

  ///////////////////////////
  // Graphics widget settings

  Bool_t DisplayTitlesEnable;
  
  string DisplayTitle, DisplayXTitle, DisplayYTitle;
  Double_t DisplayXTitleSize, DisplayXTitleOffset;
  Double_t DisplayYTitleSize, DisplayYTitleOffset;

  Bool_t DisplayTrigger, DisplayBaselineBox;
  Bool_t DisplayPSDLimits, DisplayZLEThreshold;
  Bool_t DisplayLegend, DisplayGrid;

  Bool_t DisplayXAxisInLog, DisplayYAxisInLog;

  Bool_t WaveformWithLine, WaveformWithMarkers, WaveformWithBoth;
  Bool_t SpectrumWithLine, SpectrumWithMarkers, SpectrumWithBars;

  int SpectrumRefreshRate;

  Bool_t DisplayContinuous, DisplayUpdateable, DisplayNonUpdateable;

  
  //////////////////////////////
  // Persistent storage settings
  
  Bool_t WaveformStorageEnable;
  Bool_t WaveformStoreRaw;
  Bool_t WaveformStoreEnergyData;
  Bool_t WaveformStorePSDData;

  Bool_t ObjectSaveWithTimeExtension;
  Bool_t CanvasSaveWithTimeExtension;
  
  ClassDef(AASettings, 1);
};

#endif
