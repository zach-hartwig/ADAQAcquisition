#ifndef __AAGraphics_hh__
#define __AAGraphics_hh__ 1

#include <TObject.h>

class AAGraphics : public TObject
{
public:

  AAGraphics();
  ~AAGraphics();

  ClassDef(AAGraphics, 0);
  
private:
};

  /*
  TLegend *DGScopeWaveform_Leg;
  TLine *DGScopeChannelTrigger_L[8];
  TBox *DGScopeBaselineCalcRegion_B[8];
  TLine *DGScopeSpectrumCalibration_L;
  */



#endif
