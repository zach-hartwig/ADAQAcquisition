#ifndef __AAAcquisitionManager_hh__
#define __AAAcquisitionManager_hh__ 1

#include <TObject.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TGraph.h>

#ifndef __CINT__
#include <boost/cstdint.hpp>
#endif

#include <vector>
#include <string>
using namespace std;

#include "ADAQRootClasses.hh"

#include "AATypes.hh"

class AAAcquisitionManager : public TObject
{
public:
  AAAcquisitionManager();
  ~AAAcquisitionManager();

  static AAAcquisitionManager *GetInstance();

  void StartAcquisition();
  void StopAcquisition();


  ClassDef(AAAcquisitionManager, 0);
  
private:
  static AAAcquisitionManager *TheAcquisitionManager;

  TGraph *Waveform_G[8];
  TH1F *Spectrum_H[8];

  TFile *OutputDataFile;
  TTree *WaveformTree;
  bool BranchWaveformTree;
  ADAQRootMeasParams *MeasParams;
  TObjString *MeasComment;
  bool ROOTFileOpen;

  vector<bool> UseCalibrationManager;
  vector<TGraph *> CalibrationManager;
  vector<ADAQChannelCalibrationData> CalibrationData;

  // Strings for file names, extensions
  string DataFileName, DataFileExtension;
  string SpectrumFileName, SpectrumFileExtension;
  string GraphicsFileName, GraphicsFileExtension;


};

#endif
