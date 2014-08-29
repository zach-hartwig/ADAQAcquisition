#ifndef __AASettings_hh__
#define __AASettings_hh__ 1

#include <TObject.h>

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

class AASettings : public TObject
{
public:
#ifndef __CINT__

  //////////////////////////
  // Channel widget settings
  
  bool     ChEnable[8];
  bool     ChPosPolarity[8];
  bool     ChNegPolarity[8];
  uint32_t ChVertPos[8];
  uint32_t ChDCOffset[8];
  uint32_t ChTriggerThreshold[8];
  uint32_t ChBaselineCalcMin[8];
  uint32_t ChBaselineCalcMax[8];
  uint32_t ChZSThreshold[8];
  uint32_t ChZSForward[8];
  uint32_t ChZSBackward[8];
  bool     ChZSPosLogic[8];
  bool     ChZSNegLogic[8];


  ///////////////////////////////////////
  // Acquisition control widget settings

  // Type
  bool WaveformMode;
  bool SpectrumMode;
  bool HighRateMode;
  bool UltraRateMode;

  // Trigger
  bool TriggerCoincidenceEnable;
  uint32_t TriggerCoincidenceLevel;
  uint32_t TriggerType;
  uint32_t TriggerEdge;

  // Acquisition
  uint32_t RecordLength;
  uint32_t PostTriggerSize;
  uint32_t AcquisitionTime;

  // Readout
  uint32_t MaxEventsBeforeTransfer;
  bool DataReductionEnable;
  uint32_t DataReductionFactor;

  
  ////////////////////////////////////
  // Spectrum creation widget settings

  // Histogram
  uint32_t SpectrumChannel;
  uint32_t SpectrumNumBins;
  uint32_t SpectrumMinBin;
  uint32_t SpectrumMaxBin;

  // Analysis
  bool SpectrumPulseHeight, SpectrumPulseArea;
  uint32_t SpectrumLLD, SpectrumULD;
  bool LDEnable;
  uint32_t LDChannel;

  // Calibration
  bool SpectrummCalibrationEnable;
  bool SpectrumCalibrationUseSlider;


  ///////////////////////////
  // Graphics widget settings

  bool PlotXAxisInSamples;
  bool PlotYAxisInADC;
  bool PlotLegend;
  bool PlotXAxisInLog, PlotYAxisInLog;

  
  ////////////////////////////////
  // Miscellaneous widget settings

  uint32_t SpectrumRefreshRate;

#endif

  ClassDef(AASettings, 0);
};

#endif
