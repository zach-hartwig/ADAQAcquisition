#ifndef __AASettings_hh__
#define __AASettings_hh__ 1

#include <TObject.h>

#include <vector>
#include <iostream>
using namespace std;

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

class AASettings : public TObject
{
public:
  
  AASettings(int Channels){
    ChEnable.resize(Channels);
    ChPosPolarity.resize(Channels);
    ChNegPolarity.resize(Channels);
    ChVertPos.resize(Channels);
    ChDCOffset.resize(Channels);
    ChTriggerThreshold.resize(Channels);
    ChBaselineCalcMin.resize(Channels);
    ChBaselineCalcMax.resize(Channels);
    ChZSThreshold.resize(Channels);
    ChZSForward.resize(Channels);
    ChZSBackward.resize(Channels);
    ChZSPosLogic.resize(Channels);
    ChZSNegLogic.resize(Channels);
  }
  
#ifndef __CINT__

  //////////////////////////
  // Channel widget settings
  
  vector<bool>     ChEnable;
  vector<bool>     ChPosPolarity;
  vector<bool>     ChNegPolarity;
  vector<uint32_t> ChVertPos;
  vector<uint32_t> ChDCOffset;
  vector<uint32_t> ChTriggerThreshold;
  vector<uint32_t> ChBaselineCalcMin;
  vector<uint32_t> ChBaselineCalcMax;
  vector<uint32_t> ChZSThreshold;
  vector<uint32_t> ChZSForward;
  vector<uint32_t> ChZSBackward;
  vector<bool>     ChZSPosLogic;
  vector<bool>     ChZSNegLogic;


  //////////////////////////
  // Display widget settings

  double HorizontalSliderMin, HorizontalSliderMax;
  double VerticalSliderMin, VerticalSliderMax;


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
  uint32_t PostTrigger;
  uint32_t AcquisitionTime;

  // Readout
  uint32_t EventsBeforeReadout;
  bool DataReductionEnable;
  uint32_t DataReductionFactor;
  bool ZeroSuppressionEnable;

  
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
  bool SpectrumCalibrationEnable;
  bool SpectrumCalibrationUseSlider;


  ///////////////////////////
  // Graphics widget settings

  bool DisplayTitlesEnable;
  
  string DisplayTitle, DisplayXTitle, DisplayYTitle;
  double DisplayXTitleSize, DisplayXTitleOffset;
  double DisplayYTitleSize, DisplayYTitleOffset;

  bool DisplayXAxisInSamples;
  bool DisplayYAxisInADC;
  bool DisplayLegend;
  bool DisplayXAxisInLog, DisplayYAxisInLog;

  
  ////////////////////////////////
  // Miscellaneous widget settings

 uint32_t SpectrumRefreshRate;

#endif

  ClassDef(AASettings, 0);
};

#endif
