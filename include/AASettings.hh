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
    ChDCOffset.resize(Channels);
    ChTriggerThreshold.resize(Channels);
    ChBaselineCalcMin.resize(Channels);
    ChBaselineCalcMax.resize(Channels);
    ChZLEThreshold.resize(Channels);
    ChZLEForward.resize(Channels);
    ChZLEBackward.resize(Channels);
    ChZLEPosLogic.resize(Channels);
    ChZLENegLogic.resize(Channels);
  }
  
#ifndef __CINT__

  //////////////////////////
  // Channel widget settings
  
  vector<bool>     ChEnable;
  vector<bool>     ChPosPolarity;
  vector<bool>     ChNegPolarity;
  vector<uint32_t> ChDCOffset;
  vector<uint32_t> ChTriggerThreshold;
  vector<uint32_t> ChBaselineCalcMin;
  vector<uint32_t> ChBaselineCalcMax;
  vector<uint32_t> ChZLEThreshold;
  vector<uint32_t> ChZLEForward;
  vector<uint32_t> ChZLEBackward;
  vector<bool>     ChZLEPosLogic;
  vector<bool>     ChZLENegLogic;


  //////////////////////////
  // Display widget settings

  double HorizontalSliderPtr;
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
  uint32_t AcquisitionControl;
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
  bool LDTrigger;
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

  bool DisplayLegend, DisplayGrid;
  bool DisplayXAxisInLog, DisplayYAxisInLog;

  bool WaveformWithCurve, WaveformWithMarkers, WaveformWithBoth;
  bool SpectrumWithCurve, SpectrumWithMarkers, SpectrumWithBars;

  int SpectrumRefreshRate;

  
  //////////////////////////////
  // Persistent storage settings
  
  bool WaveformStorageEnable;
  bool SpectrumSaveWithTimeExtension;
  bool CanvasSaveWithTimeExtension;

#endif

  ClassDef(AASettings, 0);
};

#endif
