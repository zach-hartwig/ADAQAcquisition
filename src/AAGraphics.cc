/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                           Copyright (C) 2012-2016                           //
//                 Zachary Seth Hartwig : All rights reserved                  //
//                                                                             //
//      The ADAQAcquisition source code is licensed under the GNU GPL v3.0.    //
//      You have the right to modify and/or redistribute this source code      //      
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at $ADAQACQUISITION/License.txt.     //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

// ROOT
#include <TGraph.h>
#include <TStyle.h>
#include <TFrame.h>
#include <TPaletteAxis.h>

// Boost
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

// C++
#include <iostream>
#include <sstream>
#include <numeric>
#include <cmath>
#include <list>

// ADAQAcquisition
#include "AAGraphics.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "ADAQDigitizer.hh"


AAGraphics *AAGraphics::TheGraphicsManager = 0;


AAGraphics *AAGraphics::GetInstance()
{ return TheGraphicsManager; }


AAGraphics::AAGraphics()
  : MaxWaveformLength(0), WaveformWidth(2), SpectrumWidth(2),
    XMin(0.), XMax(1.), YMin(0.), YMax(1.),
    BaselineStart(0), BaselineStop(1),
    WaveformGraphAxes_H(new TH1F), RateGraphAxes_H(new TH1F)
{
  if(TheGraphicsManager)
    cout << "\nError! The GraphicsManager was constructed twice!\n" << endl;
  TheGraphicsManager = this;

  // Set the default palette used to color the PSDHistogram
  gStyle->SetPalette(kBird);
  
  // Fill a vector with channel colors for plotting
  ChColor +=
    kBlue, kViolet+1, kMagenta, kPink+1,
    kRed, kOrange+2, kGreen+1, kCyan+1,
    kBlue+2, kViolet+2, kMagenta+2, kPink+3,
    kRed+2, kOrange+3, kGreen+3, kCyan+3;
  
  // Get the number of digitizer channels
  const Int_t NumDGChannels = 16;//AAVMEManager::GetInstance()->GetDGManager()->GetNumChannels();

  // Initialize channel-specific graphical objects
  for(int ch=0; ch<NumDGChannels; ch++){
    Trigger_L.push_back(new TLine);
    Trigger_L[ch]->SetLineColor(ChColor[ch]);
    Trigger_L[ch]->SetLineStyle(7);
    Trigger_L[ch]->SetLineWidth(2);

    ZLE_L.push_back(new TLine);
    ZLE_L[ch]->SetLineColor(ChColor[ch]);
    ZLE_L[ch]->SetLineStyle(10);
    ZLE_L[ch]->SetLineWidth(2);
    
    Baseline_B.push_back(new TBox);
    Baseline_B[ch]->SetFillColor(ChColor[ch]);
    Baseline_B[ch]->SetFillStyle(3002);
    Baseline_B[ch]->SetLineWidth(0);
    
    BaselineStart.push_back(0);
    BaselineStop.push_back(1);
    
    PSDTotal_B.push_back(new TBox);
    PSDTotal_B[ch]->SetFillColor(ChColor[ch]);
    PSDTotal_B[ch]->SetFillStyle(3002);

    PSDPeak_L.push_back(new TLine);
    PSDPeak_L[ch]->SetLineColor(kRed);
    PSDPeak_L[ch]->SetLineStyle(7);
    PSDPeak_L[ch]->SetLineWidth(2);

    PSDTrigger_L.push_back(new TLine);
    PSDTrigger_L[ch]->SetLineColor(kRed);
    PSDTrigger_L[ch]->SetLineStyle(7);
    PSDTrigger_L[ch]->SetLineWidth(2);

    PSDTail_L0.push_back(new TLine);
    PSDTail_L0[ch]->SetLineColor(ChColor[ch]);
    PSDTail_L0[ch]->SetLineStyle(1);
    PSDTail_L0[ch]->SetLineWidth(2);
    
    PSDTail_L1.push_back(new TLine);
    PSDTail_L1[ch]->SetLineColor(ChColor[ch]);
    PSDTail_L1[ch]->SetLineStyle(1);
    PSDTail_L1[ch]->SetLineWidth(2);

    PSDTriggerHoldoff_L.push_back(new TLine);
    PSDTriggerHoldoff_L[ch]->SetLineColor(ChColor[ch]);
    PSDTriggerHoldoff_L[ch]->SetLineStyle(7);
    PSDTriggerHoldoff_L[ch]->SetLineWidth(2);
    
    WaveformGraphs.push_back(new TGraph);
  }
  
  SpectrumCalibration_L = new TLine;
  SpectrumCalibration_L->SetLineColor(kRed);
  SpectrumCalibration_L->SetLineStyle(7);
  SpectrumCalibration_L->SetLineWidth(2);

  // Create a legend for the waveforms

  Waveform_LG = new TLegend(0.85, 0.4, 0.95, 0.92);
  Waveform_LG->SetTextSize(0.04);
  
  for(Int_t ch=0; ch<NumDGChannels; ch++){
    TLine *Line = new TLine;
    Line->SetLineColor(ChColor[ch]);
    Line->SetLineWidth(4);
    
    stringstream SS;
    SS << "Ch" << ch;
    string LineName = SS.str();
    
    Waveform_LG->AddEntry(Line, LineName.c_str(), "L");
  }

  RateGraph = NULL;
}


AAGraphics::~AAGraphics()
{;}


void AAGraphics::SetupWaveformGraphics(vector<Int_t> &WaveformLength)
{
  // While CAEN STD firmware has fixed record lengths for all channels
  // on a single digitizer, CAEN DPP-PSD firmware enables different
  // record lengths for all channels. Therefore, if plotting multiple
  // channels with DPP-PSD, we must account for this and ensure the
  // X-axis spans the longest of the enabled channel record lengths

  MaxWaveformLength = 0;
  vector<Int_t>::iterator It = WaveformLength.begin();
  for(; It!=WaveformLength.end(); It++){
    if((*It) > MaxWaveformLength)
      MaxWaveformLength = (*It);
  }

  ADAQDigitizer *DGManager = AAVMEManager::GetInstance()->GetDGManager();

  // Setup the baseline calculation start/stop values here a single
  // time since these don't change during waveform acquisition

  // STD firmware: baseline width, position set in absolute sample numbes
  // PSD firwmare: baseline width set by user in fixed samples; position in
  //               time is relative to pregate setting
  
  Int_t NumDGChannels = DGManager->GetNumChannels();
  
  for(Int_t ch=0; ch<DGManager->GetNumChannels(); ch++){
    
    if(TheSettings->STDFirmware){
      BaselineStart[ch] = TheSettings->ChBaselineCalcMin[ch];
      BaselineStop[ch] = TheSettings->ChBaselineCalcMax[ch];
    }
    else if(TheSettings->PSDFirmware){
      
      Int_t BaselineSamples = 0;
      Int_t BaselineSelection = TheSettings->ChBaselineSamples[ch];
      
      ZBoardType DGType = AAVMEManager::GetInstance()->GetDGManager()->GetBoardType();
      
      if(DGType == zV1720 or DGType == zDT5720 or
	 DGType == zDT5790M or DGType == zDT5790N or DGType == zDT5790P){
	  
	switch(BaselineSelection){
	case 1:
	  BaselineSamples = 8;
	  break;
	case 2:
	  BaselineSamples = 32;
	  break;
	case 3:
	  BaselineSamples = 128;
	  break;
	default:
	  break;
	}
      }
      else if(DGType == zV1725 or DGType == zDT5730){
	  
	switch(BaselineSelection){
	case 1:
	  BaselineSamples = 16;
	  break;
	case 2:
	  BaselineSamples = 64;
	  break;
	case 3:
	  BaselineSamples = 256;
	  break;
	case 4:
	  BaselineSamples = 1024;
	  break;
	default:
	  break;
	}
      }
      BaselineStop[ch] = TheSettings->ChPreTrigger[ch] - TheSettings->ChGateOffset[ch] - 1;
      BaselineStart[ch] = BaselineStop[ch] - BaselineSamples;
    }
  }

  Time.clear();
  for(int t=0; t<MaxWaveformLength; t++)
    Time.push_back(t);
  
  if(TheSettings->DisplayTitlesEnable){
    Title = TheSettings->DisplayTitle;
    XTitle = TheSettings->DisplayXTitle;
    YTitle = TheSettings->DisplayYTitle;
    
    XSize = TheSettings->DisplayXTitleSize;
    XOffset = TheSettings->DisplayXTitleOffset;

    YSize = TheSettings->DisplayYTitleSize;
    YOffset = TheSettings->DisplayYTitleOffset;
  }
  else{
    Title = "Digitized waveform";
    XTitle = "Time [sample]";
    YTitle = "Voltage [ADC]";
    
    XSize = YSize = 0.05;
    XOffset = 1.1;
    YOffset = 1.2;
  }

  // In order to maximize the drawing efficiency of waveforms and to
  // account for the possibility that the number of possible data
  // channels might change between runs, the vector containing the
  // TGraphs is operated on here (recall, this method is only called
  // once at the beginning of acquisition) in the following order:
  //
  // 0. Previous TGraph objects are deleted to prevent memory leak
  // 1. New TGraph objects are created for all channels
  // 3. Static graphical attributes are set for each channel's TGraph
  
  vector<TGraph *>::iterator it = WaveformGraphs.begin();
  for(; it!=WaveformGraphs.end(); it++)
    delete (*it);
  
  WaveformGraphs.clear();
  WaveformGraphs.resize(NumDGChannels);
  
  for(Int_t ch=0; ch<NumDGChannels; ch++){
    
    // Create a new TGraph representing the waveform
    WaveformGraphs[ch] = new TGraph;
    
    // Set the static waveform graphical options
    WaveformGraphs[ch]->SetLineColor(ChColor[ch]);
    WaveformGraphs[ch]->SetLineWidth(WaveformWidth);
    WaveformGraphs[ch]->SetMarkerStyle(24);
    WaveformGraphs[ch]->SetMarkerSize(0.75);
    WaveformGraphs[ch]->SetMarkerColor(ChColor[ch]);
    WaveformGraphs[ch]->SetFillColor(ChColor[ch]);
  }
  
  // Delete and recreate a TH1F object that is used to create the X
  // and Y axes for plotting the waveforms. The title/xtitle/ytitle
  // and other graphical options should be set here.
  
  delete WaveformGraphAxes_H;
  WaveformGraphAxes_H = new TH1F("WaveformGraphAxes_H",
				 "A TH1F used to create X and Y axes for waveform plotting",
				 100, 0, MaxWaveformLength);
  
  // Set the waveform title and axes properties
  WaveformGraphAxes_H->SetTitle(Title.c_str());
  
  WaveformGraphAxes_H->GetXaxis()->SetTitle(XTitle.c_str());
  WaveformGraphAxes_H->GetXaxis()->SetTitleSize(XSize);
  WaveformGraphAxes_H->GetXaxis()->SetTitleOffset(XOffset);
  WaveformGraphAxes_H->GetXaxis()->SetLabelSize(XSize);
  WaveformGraphAxes_H->GetXaxis()->SetRangeUser(0, MaxWaveformLength);
  
  WaveformGraphAxes_H->GetYaxis()->SetTitle(YTitle.c_str());
  WaveformGraphAxes_H->GetYaxis()->SetTitleSize(YSize);
  WaveformGraphAxes_H->GetYaxis()->SetTitleOffset(YOffset);
  WaveformGraphAxes_H->GetYaxis()->SetLabelSize(YSize);

  WaveformGraphAxes_H->SetStats(false);
}


void AAGraphics::PlotWaveforms(vector<vector<uint16_t> > &Waveforms, 
			       vector<Int_t> &WaveformLength)
{
  Int_t NumGraphs = 0;

  Int_t NumDGChannels = AAVMEManager::GetInstance()->GetDGManager()->GetNumChannels();
  
  for(int ch=0; ch<NumDGChannels; ch++){
    
    if(!TheSettings->ChEnable[ch])
      continue;
    
    // Efficienctly allocate the channel voltage into a vector<int>
    vector<Int_t> Voltage (Waveforms[ch].begin(), Waveforms[ch].end());

    // Zero length encoding waveform: a vector representing time (the
    // X-axis) must be created dynamically to account for the variable
    // time nature of the ZLE waveform. Do this efficiently with
    // std::iota methods

    if(TheSettings->ZeroSuppressionEnable){
      WaveformLength[ch] = Voltage.size();
      Time.resize(WaveformLength[ch]);
      iota(begin(Time), end(Time), 0);
    }
    
    // Prevent plotting if there is no waveform values to plot
    if(Voltage.size() == 0)
      return;
    
    // Set the horiz. and vert. min/max ranges of the waveform.  Note
    // the max value is the max digitizer bit value in units of ADC

    XMin = MaxWaveformLength * TheSettings->HorizontalSliderMin;
    XMax = MaxWaveformLength * TheSettings->HorizontalSliderMax;
    WaveformGraphs[ch]->GetXaxis()->SetRangeUser(XMin, XMax);

    (TheSettings->DisplayXAxisInLog) ? 
      gPad->SetLogx(true) : gPad->SetLogx(false);
    
    Int_t AbsoluteMax = AAVMEManager::GetInstance()->GetDGManager()->GetMaxADCBit();
    YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
    YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
    
    if(TheSettings->DisplayYAxisInLog){
      if(YMin == 0) YMin = 1;
      gPad->SetLogy(true);
    }
    else
      gPad->SetLogy(false);
    
    // The WaveformGraphAxes_H object creates the X and Y axes upon
    // which the TGraph waveform objects are plotted. A TH1F is used
    // because (a)the TAxis::SetRangeUser() methods do NOT appear to
    // work when using the TGraph::DrawGraph() method and (b) to
    // refresh the canvas between successive triggers. This is done
    // such that we do NOT need to delete/recreate the TGraph object
    // via dynamic memory allocation every time we plot (as was done
    // previously). This should have the effect of substantially
    // boosting acquisition performance at high rates due to decreased
    // CPU/memory time spent plotting waveforms.
    
    // If this is the first channel being drawn, dynamically set the X
    // and Y ranges based on the horizontal and vertical double slider
    // positions and draw the axes
    if(NumGraphs == 0){
      WaveformGraphAxes_H->GetXaxis()->SetRangeUser(XMin,XMax);
      WaveformGraphAxes_H->SetMinimum(YMin);
      WaveformGraphAxes_H->SetMaximum(YMax);
      WaveformGraphAxes_H->Draw("");
    }

    // Set the TGraph waveform plotting options
    TString DrawOptions;
    if(TheSettings->WaveformWithLine)
      DrawOptions += "L";
    else if(TheSettings->WaveformWithMarkers)
      DrawOptions += "P";
    else
      DrawOptions += "PL";

    // Draw the graph with the new time/voltage values
    WaveformGraphs[ch]->DrawGraph(WaveformLength[ch], 
				  &Time[0], 
				  &Voltage[0],
				  DrawOptions);
    
    NumGraphs++;
  }
  
  if(TheSettings->DisplayLegend)
    Waveform_LG->Draw();
  
  (TheSettings->DisplayGrid) ? gPad->SetGrid(true, true) : gPad->SetGrid(false, false);
}


void AAGraphics::DrawWaveformGraphics(vector<double> &BaselineValue,
				      vector<Int_t> &PeakPosition,
				      vector<int> &PSDTotalAbsStart,
				      vector<int> &PSDTotalAbsStop,
				      vector<int> &PSDTailAbsStart,
				      vector<int> &PSDTailAbsStop)
{
  for(int ch=0; ch<TheSettings->ChEnable.size(); ch++){
    
    if(!TheSettings->ChEnable[ch])
      continue;

    if(TheSettings->DisplayTrigger){

      // STD firmware: trigger value is in absolute ADC units
      // PSD firmware: trigger value relative to baseline in ADC units
      
      Double_t Trigger = 0.;
      if(TheSettings->PSDFirmware){
	if(TheSettings->ChPosPolarity[ch])
	  Trigger = TheSettings->ChTriggerThreshold[ch] + BaselineValue[ch];
	else if(TheSettings->ChNegPolarity[ch])
	  Trigger = BaselineValue[ch] - TheSettings->ChTriggerThreshold[ch];
      }
      else
	Trigger = TheSettings->ChTriggerThreshold[ch];
      
      Trigger_L[ch]->DrawLine(XMin, Trigger, XMax, Trigger);

      if(TheSettings->PSDFirmware){
	PSDTrigger_L[ch]->DrawLine(TheSettings->ChPreTrigger[ch]-1,
				   YMin,
				   TheSettings->ChPreTrigger[ch]-1,
				   YMax);
      }
    }
    
    if(TheSettings->DisplayBaselineBox){
      Double_t BaselineWidth = (YMax-YMin)*0.03;
      
      Baseline_B[ch]->DrawBox(BaselineStart[ch],
			      BaselineValue[ch] - BaselineWidth,
			      BaselineStop[ch],
			      BaselineValue[ch] + BaselineWidth);
    }
    
    if(TheSettings->DisplayPSDLimits){

      PSDTotal_B[ch]->DrawBox(PSDTotalAbsStart[ch],
			      YMin,
			      PSDTotalAbsStop[ch],
			      YMax);

      if(TheSettings->STDFirmware)
	PSDPeak_L[ch]->DrawLine(PeakPosition[ch],
				YMin,
				PeakPosition[ch],
				YMax);
      
      PSDTail_L0[ch]->DrawLine(PSDTailAbsStart[ch],
			       YMin,
			       PSDTailAbsStart[ch],
			       YMax);

      PSDTail_L1[ch]->DrawLine(PSDTailAbsStop[ch],
			       YMin,
			       PSDTailAbsStop[ch],
			       YMax);

      PSDTriggerHoldoff_L[ch]->DrawLine(PSDTotalAbsStop[ch]+TheSettings->PSDTriggerHoldoff,
					YMin,
					PSDTotalAbsStop[ch]+TheSettings->PSDTriggerHoldoff,
					YMax);
    }

    if(TheSettings->DisplayZLEThreshold)
      ZLE_L[ch]->DrawLine(XMin,
			  TheSettings->ChZLEThreshold[ch],
			  XMax,
			  TheSettings->ChZLEThreshold[ch]);
  }
  TheCanvas_C->Update();
}

void AAGraphics::SetupRateGraphics()
{
  if(TheSettings->DisplayTitlesEnable){
    Title = TheSettings->DisplayTitle;
    XTitle = TheSettings->DisplayXTitle;
    YTitle = TheSettings->DisplayYTitle;
    
    XSize = TheSettings->DisplayXTitleSize;
    XOffset = TheSettings->DisplayXTitleOffset;

    YSize = TheSettings->DisplayYTitleSize;
    YOffset = TheSettings->DisplayYTitleOffset;
  }
  else{
    Title = "Trigger rate";
    string Unit = TheSettings->SpectrumCalibrationUnit;
    XTitle = "Run time [s]";

    YTitle = "Triggers/s";
    
    XSize = YSize = 0.05;
    XOffset = 1.1;
    YOffset = 1.2;
  }

	if (RateGraph)
		delete RateGraph;

  if (!RateGraph)
    RateGraph =  new TGraph();

  delete RateGraphAxes_H;
  RateGraphAxes_H = new TH1F("RateGraphAxes_H",
				 "A TH1F used to create X and Y axes for rate plotting",
				 100, 0, MaxWaveformLength);
  
  // Set the waveform title and axes properties
  RateGraphAxes_H->SetTitle(Title.c_str());
  
  RateGraphAxes_H->GetXaxis()->SetTitle(XTitle.c_str());
  RateGraphAxes_H->GetXaxis()->SetTitleSize(XSize);
  RateGraphAxes_H->GetXaxis()->SetTitleOffset(XOffset);
  RateGraphAxes_H->GetXaxis()->SetLabelSize(XSize);
  RateGraphAxes_H->GetXaxis()->SetRangeUser(0, MaxWaveformLength);
  
  RateGraphAxes_H->GetYaxis()->SetTitle(YTitle.c_str());
  RateGraphAxes_H->GetYaxis()->SetTitleSize(YSize);
  RateGraphAxes_H->GetYaxis()->SetTitleOffset(YOffset);
  RateGraphAxes_H->GetYaxis()->SetLabelSize(YSize);

  RateGraphAxes_H->SetStats(false);
}

void AAGraphics::SetupSpectrumGraphics()
{
  if(TheSettings->DisplayTitlesEnable){
    Title = TheSettings->DisplayTitle;
    XTitle = TheSettings->DisplayXTitle;
    YTitle = TheSettings->DisplayYTitle;
    
    XSize = TheSettings->DisplayXTitleSize;
    XOffset = TheSettings->DisplayXTitleOffset;

    YSize = TheSettings->DisplayYTitleSize;
    YOffset = TheSettings->DisplayYTitleOffset;
  }
  else{
    Title = "Pulse spectrum";
    
    Int_t Channel = TheSettings->SpectrumChannel;
    
    Bool_t Calibrated = AAAcquisitionManager::GetInstance()->GetCalibrationEnable(Channel);

    if(Calibrated){
      string Unit = TheSettings->SpectrumCalibrationUnit;
      XTitle = "Energy deposited [" + Unit + "]";
    }
    else{
      if(TheSettings->SpectrumPulseHeight)
        XTitle = "Pulse height [ADC]";
      else if(TheSettings->SpectrumPulseArea)
        XTitle = "Pulse area [ADC]";
    }
    YTitle = "Counts";
    
    XSize = YSize = 0.05;
    XOffset = 1.1;
    YOffset = 1.2;
  }

}

void AAGraphics::PlotRate(Double_t tss)
{
  Int_t Channel = TheSettings->RateChannel;
  std::list<unsigned int> * data = AAAcquisitionManager::GetInstance()->GetRateList(Channel);

  // Plotting arrays filled with the data from the list pointer
  Double_t ta[data->size()];
  Double_t ra[data->size()];

  unsigned int ci = 0;
  Double_t AbsoluteMax = 0;
  for (std::list<unsigned int>::iterator it=data->begin(); it != data->end(); ++it){
    ta[ci] = ci*TheSettings->RateIntegrationPeriod + tss;
    ra[ci] = ((Double_t)*it)/TheSettings->RateIntegrationPeriod;
    if (ra[ci]>AbsoluteMax) AbsoluteMax = 1.05*ra[ci];
    ci++;
  }
  

  RateGraph->SetLineColor(ChColor[Channel]);
  RateGraph->SetLineWidth(SpectrumWidth);
  RateGraph->SetMarkerStyle(24);
  RateGraph->SetMarkerColor(ChColor[Channel]);
  RateGraph->SetMarkerSize(0.75);
  RateGraph->SetFillColor(ChColor[Channel]);

//  // Set spectrum axes range and lin/log 

  XMin = ta[data->size()-1] * TheSettings->HorizontalSliderMin;
  XMax = ta[data->size()-1] * TheSettings->HorizontalSliderMax;
  
  (TheSettings->DisplayXAxisInLog) ? 
    gPad->SetLogx(true) : gPad->SetLogx(false);
  
  // Double_t AbsoluteMax = RateGraph->GetMaximum() * 1.05;
  YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
  YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
  
  if(TheSettings->DisplayYAxisInLog){
    if(YMin == 0) YMin = 1;
    gPad->SetLogy(true);
  }
  else 
    gPad->SetLogy(false);

	RateGraphAxes_H->GetXaxis()->SetRangeUser(XMin,XMax);
	RateGraphAxes_H->SetMinimum(YMin);
	RateGraphAxes_H->SetMaximum(YMax);
	RateGraphAxes_H->Draw("");

	RateGraph->SetTitle("");
  RateGraph->DrawGraph(data->size(),ta,ra,"ALP");
  RateGraph->GetXaxis()->SetRangeUser(XMin, XMax);
  
  // Set plot and axis title text properties


  (TheSettings->DisplayGrid) ? gPad->SetGrid(true, true) : gPad->SetGrid(false, false);
  
  // If calibration is enabled the draw a vertical line corresponding
  // to the current pulse value selected by the triple slider pointer

  TheCanvas_C->Update();
}

void AAGraphics::PlotSpectrum(TH1F *Spectrum_H)
{
  Int_t Channel = TheSettings->SpectrumChannel;

  Spectrum_H->SetLineColor(ChColor[Channel]);
  Spectrum_H->SetLineWidth(SpectrumWidth);
  Spectrum_H->SetMarkerStyle(24);
  Spectrum_H->SetMarkerColor(ChColor[Channel]);
  Spectrum_H->SetMarkerSize(0.75);
  Spectrum_H->SetFillColor(ChColor[Channel]);
  
  if(TheSettings->SpectrumWithLine){
    Spectrum_H->SetFillStyle(0);
    Spectrum_H->Draw("");
  }
  else if(TheSettings->SpectrumWithMarkers)
    Spectrum_H->Draw("E1");
  else
    Spectrum_H->Draw("B");

  // Set spectrum axes range and lin/log 

  XMin = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMin;
  XMax = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMax;
  Spectrum_H->GetXaxis()->SetRangeUser(XMin, XMax);
  
  (TheSettings->DisplayXAxisInLog) ? 
    gPad->SetLogx(true) : gPad->SetLogx(false);
  
  Int_t AbsoluteMax = Spectrum_H->GetBinContent(Spectrum_H->GetMaximumBin()) * 1.05;
  YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
  YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
  
  if(TheSettings->DisplayYAxisInLog){
    if(YMin == 0) YMin = 1;
    gPad->SetLogy(true);
  }
  else 
    gPad->SetLogy(false);
  
  Spectrum_H->SetMinimum(YMin);
  Spectrum_H->SetMaximum(YMax);

  // Set plot and axis title text properties

  Spectrum_H->SetTitle(Title.c_str());
  
  Spectrum_H->GetXaxis()->SetTitle(XTitle.c_str());
  Spectrum_H->GetXaxis()->SetTitleSize(XSize);
  Spectrum_H->GetXaxis()->SetTitleOffset(XOffset);
  Spectrum_H->GetXaxis()->SetLabelSize(XSize);

  Spectrum_H->GetYaxis()->SetTitle(YTitle.c_str());
  Spectrum_H->GetYaxis()->SetTitleSize(YSize);
  Spectrum_H->GetYaxis()->SetTitleOffset(YOffset);
  Spectrum_H->GetYaxis()->SetLabelSize(YSize);

  (TheSettings->DisplayLegend) ? Spectrum_H->SetStats(true) : Spectrum_H->SetStats(false);
  (TheSettings->DisplayGrid) ? gPad->SetGrid(true, true) : gPad->SetGrid(false, false);
  
  // If calibration is enabled the draw a vertical line corresponding
  // to the current pulse value selected by the triple slider pointer

  if(TheSettings->SpectrumCalibrationEnable){
    Double_t PulseValue = TheSettings->SpectrumMaxBin *
      TheSettings->HorizontalSliderPtr;
    
    SpectrumCalibration_L->DrawLine(PulseValue,
				    YMin,
				    PulseValue,
				    YMax);
  }
  TheCanvas_C->Update();
}


void AAGraphics::SetupPSDHistogramGraphics()
{
  if(TheSettings->DisplayTitlesEnable){
    Title = TheSettings->DisplayTitle;
    XTitle = TheSettings->DisplayXTitle;
    YTitle = TheSettings->DisplayYTitle;
    
    XSize = TheSettings->DisplayXTitleSize;
    XOffset = TheSettings->DisplayXTitleOffset;
    
    YSize = TheSettings->DisplayYTitleSize;
    YOffset = TheSettings->DisplayYTitleOffset;
  }
  else{
    Title = "Pulse shape discrimination (PSD)";
    XTitle = "Total integral [ADC]";
    if(TheSettings->PSDYAxisTail)
      YTitle = "Tail integral [ADC]";
    else
      YTitle = "(Tail / Total) integrals [None]";
    
    XSize = YSize = 0.05;
    XOffset = 1.1;
    YOffset = 1.2;
  }
}

void AAGraphics::PlotPSDHistogram(TH2F *PSDHistogram_H)
{
  Int_t Channel = TheSettings->PSDChannel;
  
  // Draw the PSDHistogram and prevent the user from inducing segfaults
  // upon moving the color axis
  PSDHistogram_H->Draw("COLZ");
  gPad->Update();
  TPaletteAxis *palette = (TPaletteAxis*)
    PSDHistogram_H->GetListOfFunctions()->FindObject("palette");
  palette->SetBit(TBox::kCannotMove);
  
  XMin = TheSettings->PSDTotalMaxBin * TheSettings->HorizontalSliderMin;
  XMax = TheSettings->PSDTotalMaxBin * TheSettings->HorizontalSliderMax;
  PSDHistogram_H->GetXaxis()->SetRangeUser(XMin, XMax);
  
  (TheSettings->DisplayXAxisInLog) ? 
    gPad->SetLogx(true) : gPad->SetLogx(false);
  
  YMin = TheSettings->PSDTailMaxBin * TheSettings->VerticalSliderMin;
  YMax = TheSettings->PSDTailMaxBin * TheSettings->VerticalSliderMax;
  PSDHistogram_H->GetYaxis()->SetRangeUser(YMin, YMax);

  if(TheSettings->DisplayYAxisInLog){
    if(YMin == 0) YMin = 1;
    gPad->SetLogy(true);
  }
  else 
    gPad->SetLogy(false);

  PSDHistogram_H->SetTitle(Title.c_str());
  
  PSDHistogram_H->GetXaxis()->SetTitle(XTitle.c_str());
  PSDHistogram_H->GetXaxis()->SetTitleSize(XSize);
  PSDHistogram_H->GetXaxis()->SetTitleOffset(XOffset);
  PSDHistogram_H->GetXaxis()->SetLabelSize(XSize);

  PSDHistogram_H->GetYaxis()->SetTitle(YTitle.c_str());
  PSDHistogram_H->GetYaxis()->SetTitleSize(YSize);
  PSDHistogram_H->GetYaxis()->SetTitleOffset(YOffset);
  PSDHistogram_H->GetYaxis()->SetLabelSize(YSize);

  (TheSettings->DisplayLegend) ? PSDHistogram_H->SetStats(true) : PSDHistogram_H->SetStats(false);
  (TheSettings->DisplayGrid) ? gPad->SetGrid(true, true) : gPad->SetGrid(false, false);

  TheCanvas_C->Update();
}


void AAGraphics::PlotCalibration(int Channel)
{
  TGraph *CalibrationCurve = AAAcquisitionManager::GetInstance()->
    GetCalibrationCurve(Channel);

  stringstream SS;
  SS << "Calibration curve Ch[" << Channel << "]";
  string Title = SS.str();

  TCanvas *CalibrationCanvas_C = new TCanvas("CalibrationCanvas_C",
					     "Channel calibration curve",
					     0, 0, 700, 500);
  
  CalibrationCurve->SetTitle(Title.c_str());
  CalibrationCurve->GetXaxis()->SetTitle("Pulse unit [ADC]");
  CalibrationCurve->GetXaxis()->SetTitleSize(0.05);
  CalibrationCurve->GetXaxis()->SetTitleOffset(1.1);
  CalibrationCurve->GetXaxis()->SetLabelSize(0.05);
  CalibrationCurve->GetXaxis()->SetNdivisions(505);
  CalibrationCurve->GetYaxis()->SetTitle("Energy");
  CalibrationCurve->GetYaxis()->SetTitleSize(0.05);
  CalibrationCurve->GetYaxis()->SetTitleOffset(1.2);
  CalibrationCurve->GetYaxis()->SetLabelSize(0.05);
  CalibrationCurve->SetMarkerSize(2);
  CalibrationCurve->SetMarkerStyle(22);
  CalibrationCurve->Draw("ALP");
  
  CalibrationCanvas_C->Update();
}
