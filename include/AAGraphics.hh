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

#ifndef __AAGraphics_hh__
#define __AAGraphics_hh__ 1

#include <TObject.h>
#include <TLegend.h>
#include <TLine.h>
#include <TBox.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TGraph.h>

#include <vector>
using namespace std;

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

#include "AASettings.hh"

class AAGraphics : public TObject
{
public:

  AAGraphics();
  ~AAGraphics();

  static AAGraphics *GetInstance();

  void SetCanvasPointer(TCanvas *C) {TheCanvas_C = C;}
  void SetSettingsPointer(AASettings *TS) {TheSettings = TS;}
  
  void SetupWaveformGraphics(vector<Int_t> &);
  
#ifndef __CINT__
  //void PlotWaveforms(vector<vector<uint16_t> > &, int, vector<double> &;
  void PlotWaveforms(vector<vector<uint16_t> > &,
		     vector<Int_t> &);
#endif
  void DrawWaveformGraphics(vector<Double_t> &, 
			    vector<Int_t> &,
			    vector<Int_t> &,
			    vector<Int_t> &,
			    vector<Int_t> &,
			    vector<Int_t> &);

  void SetupSpectrumGraphics();
  void PlotSpectrum(TH1F *);
  
  void SetupPSDHistogramGraphics();
  void PlotPSDHistogram(TH2F *);
  
  void PlotCalibration(int);

  ClassDef(AAGraphics, 1);
  
private:
  static AAGraphics *TheGraphicsManager;

  TLegend * Waveform_LG;
  vector<TLine *> Trigger_L, ZLE_L;

  vector<TLine *> PSDPeak_L, PSDTrigger_L;
  vector<TLine *> PSDTail_L0, PSDTail_L1;
  vector<TLine *> PSDTriggerHoldoff_L;
  vector<TBox *> Baseline_B, PSDTotal_B;
  
  TLine *Spectrum_L, *SpectrumCalibration_L;

  TCanvas *TheCanvas_C;

  vector<Int_t> ChColor;

  Int_t MaxWaveformLength, WaveformWidth, SpectrumWidth;

  AASettings *TheSettings;

  vector<Int_t> Time;

  string Title, XTitle, YTitle;
  Double_t XSize, YSize, XOffset, YOffset;

  Double_t XMin, XMax, YMin, YMax;

  vector<Int_t> BaselineStart, BaselineStop;
  
  vector<TGraph *> WaveformGraphs;
  TH1F *WaveformGraphAxes_H;
};

#endif
