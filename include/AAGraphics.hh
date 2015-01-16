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
  
  void SetupWaveformGraphics(int);
  void SetupSpectrumGraphics();
  
#ifndef __CINT__
  //void PlotWaveforms(vector<vector<uint16_t> > &, int, vector<double> &;
  void PlotWaveforms(vector<vector<uint16_t> > &, int);
#endif
  void DrawWaveformGraphics(vector<Double_t> &, 
			    vector<Int_t> &,
			    vector<Int_t> &,
			    vector<Int_t> &,
			    vector<Int_t> &);

  void PlotSpectrum(TH1F *);
  void PlotCalibration(int);


  ClassDef(AAGraphics, 1);
  
private:
  static AAGraphics *TheGraphicsManager;

  TLegend * Waveform_LG;
  vector<TLine *> Trigger_L, ZLE_L;
  vector<TBox *> Baseline_B, PSDTotal_B, PSDTail_B;

  TLine *Spectrum_L, *SpectrumCalibration_L;

  TCanvas *TheCanvas_C;

  vector<int> ChColor;

  int WaveformWidth, SpectrumWidth;

  AASettings *TheSettings;

  vector<int> Time;

  string Title, XTitle, YTitle;
  double XSize, YSize, XOffset, YOffset;

  double XMin, XMax, YMin, YMax;

  vector<TGraph *> WaveformGraphs;
};




#endif
