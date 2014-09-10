#ifndef __AAGraphics_hh__
#define __AAGraphics_hh__ 1

#include <TObject.h>
#include <TLegend.h>
#include <TLine.h>
#include <TBox.h>
#include <TCanvas.h>
#include <TH1F.h>

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

#ifndef __CINT__
  void PlotWaveforms(vector<vector<Short_t> > &, int, vector<double> &);
#endif
  void PlotSpectrum(vector<TH1F *> &);
  void PlotCalibration();

  ClassDef(AAGraphics, 0);
  
private:
  static AAGraphics *TheGraphicsManager;

  TLegend * Waveform_LG;
  vector<TLine *> Trigger_L;
  vector<TBox *> Baseline_B;

  TLine *Spectrum_L, *SpectrumCalibration_L;

  TCanvas *TheCanvas_C;

  vector<int> ChColor;

  int WaveformWidth, SpectrumWidth;

  AASettings *TheSettings;

  vector<int> Time;

  string Title, XTitle, YTitle;
  double XSize, YSize, XOffset, YOffset;
};




#endif
