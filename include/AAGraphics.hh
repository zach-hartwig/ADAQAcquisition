#ifndef __AAGraphics_hh__
#define __AAGraphics_hh__ 1

#include <TObject.h>
#include <TLegend.h>
#include <TLine.h>
#include <TBox.h>

#include <vector>
using namespace std;

class AAGraphics : public TObject
{
public:

  AAGraphics();
  ~AAGraphics();

  static AAGraphics *GetInstance();

  void PlotWaveform();
  void PlotSpectrum();
  void PlotCalibration();

  ClassDef(AAGraphics, 0);
  
private:
  static AAGraphics *TheGraphicsManager;

  vector<TLegend *> Waveform_LG;
  vector<TLine *> ChTrigger_L;
  vector<TBox *> ChBaselineCalc_B;

  TLine *SpectrumCalibration_L;

};

  /*
  TLegend *DGScopeWaveform_Leg;
  TLine *DGScopeChannelTrigger_L[8];
  TBox *DGScopeBaselineCalcRegion_B[8];
  TLine *DGScopeSpectrumCalibration_L;
  */



#endif
