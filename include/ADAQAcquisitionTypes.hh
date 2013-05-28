#ifndef __ADAQAcquisitionTypes_hh__
#define __ADAQAcquisitionTypes_hh__ 1

struct ADAQChannelCalibrationData{
  vector<int> PointID;
  vector<double> Energy;
  vector<double> PulseUnit;
};

#endif
