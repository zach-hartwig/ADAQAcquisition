#include <TGraph.h>
#include <TStyle.h>
#include <TFrame.h>

#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

#include <iostream>
#include <sstream>

#include "AAGraphics.hh"
#include "AAVMEManager.hh"
#include "AAAcquisitionManager.hh"
#include "ADAQDigitizer.hh"


AAGraphics *AAGraphics::TheGraphicsManager = 0;


AAGraphics *AAGraphics::GetInstance()
{ return TheGraphicsManager; }


AAGraphics::AAGraphics()
  : WaveformWidth(2), SpectrumWidth(2)
{
  if(TheGraphicsManager)
    cout << "\nError! The GraphicsManager was constructed twice!\n" << endl;
  TheGraphicsManager = this;

  // Fill a vector with channel colors for plotting
  
  ChColor += kBlue, kRed, kGreen+1, kYellow-3,
    kOrange+7, kAzure+7, kViolet-5, kMagenta;

  // Initialize lines and boxes for plotting

  for(int ch=0; ch<8; ch++){
    Trigger_L.push_back(new TLine);
    Trigger_L[ch]->SetLineColor(ChColor[ch]);
    Trigger_L[ch]->SetLineStyle(7);
    Trigger_L[ch]->SetLineWidth(2);
    
    Baseline_B.push_back(new TBox);
    Baseline_B[ch]->SetFillColor(ChColor[ch]);
    Baseline_B[ch]->SetFillStyle(3002);
    Baseline_B[ch]->SetLineWidth(0);
  }

  SpectrumCalibration_L = new TLine;
  SpectrumCalibration_L->SetLineColor(kRed);
  SpectrumCalibration_L->SetLineStyle(7);
  SpectrumCalibration_L->SetLineWidth(2);

  // Create a legend for the waveforms

  Waveform_LG = new TLegend(0.85, 0.5, 0.95, 0.92);
  Waveform_LG->SetTextSize(0.05);

  for(int ch=0; ch<8; ch++){
    TLine *Line = new TLine;
    Line->SetLineColor(ChColor[ch]);
    Line->SetLineWidth(4);

    stringstream SS;
    SS << "Ch" << ch;
    string LineName = SS.str();
    
    Waveform_LG->AddEntry(Line, LineName.c_str(), "L");
  }
}


AAGraphics::~AAGraphics()
{;}


void AAGraphics::SetupWaveformGraphics(int WaveformLength)
{
  Time.clear();
  for(int t=0; t<WaveformLength; t++)
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
}


void AAGraphics::PlotWaveforms(vector<vector<Short_t> > &Waveforms, 
			       int WaveformLength,
			       vector<double> &BaselineValue)
{
  vector<TGraph *> WaveformGraphs;
  for(int ch=0; ch<TheSettings->ChEnable.size(); ch++){
    
    if(!TheSettings->ChEnable[ch])
      continue;

    vector<int> Voltage (Waveforms[ch].begin(), Waveforms[ch].end());

    TGraph *Waveform_G = new TGraph(WaveformLength, &Time[0], &Voltage[0]);
    WaveformGraphs.push_back(Waveform_G);

    Waveform_G->Draw("AL");

    // Set the horiz. and vert. min/max ranges of the waveform.  Note
    // the max value is the max digitizer bit value in units of ADC

    double XMin = WaveformLength * TheSettings->HorizontalSliderMin;
    double XMax = WaveformLength * TheSettings->HorizontalSliderMax;
    Waveform_G->GetXaxis()->SetRangeUser(XMin, XMax);

    (TheSettings->DisplayXAxisInLog) ? 
      gPad->SetLogx(true) : gPad->SetLogx(false);
    
    int AbsoluteMax = AAVMEManager::GetInstance()->GetDGManager()->GetMaxBit();
    double YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
    double YMax = AbsoluteMax * TheSettings->VerticalSliderMax;

    if(TheSettings->DisplayYAxisInLog){
      if(YMin == 0) YMin = 1;
      gPad->SetLogy(true);
    }
    else
      gPad->SetLogy(false);

    Waveform_G->GetYaxis()->SetRangeUser(YMin, YMax);

    // Set the waveform line properties

    Waveform_G->SetLineColor(ChColor[ch]);
    Waveform_G->SetLineWidth(WaveformWidth);

    // Set the waveform title and axes properties
    Waveform_G->SetTitle(Title.c_str());

    Waveform_G->GetXaxis()->SetTitle(XTitle.c_str());
    Waveform_G->GetXaxis()->SetTitleSize(XSize);
    Waveform_G->GetXaxis()->SetTitleOffset(XOffset);
    Waveform_G->GetXaxis()->SetLabelSize(XSize);

    Waveform_G->GetYaxis()->SetTitle(YTitle.c_str());
    Waveform_G->GetYaxis()->SetTitleSize(YSize);
    Waveform_G->GetYaxis()->SetTitleOffset(YOffset);
    Waveform_G->GetYaxis()->SetLabelSize(YSize);

    // Draw additional graphical objects on top of the waveform
    
    Trigger_L[ch]->DrawLine(XMin,
			    TheSettings->ChTriggerThreshold[ch],
			    XMax,
			    TheSettings->ChTriggerThreshold[ch]);

    double BaselineWidth = (YMax-YMin)*0.03;
    
    Baseline_B[ch]->DrawBox(TheSettings->ChBaselineCalcMin[ch],
			    BaselineValue[ch] - BaselineWidth,
			    TheSettings->ChBaselineCalcMax[ch],
			    BaselineValue[ch] + BaselineWidth);
  }
  
  if(TheSettings->DisplayLegend)
    Waveform_LG->Draw();
  
  (TheSettings->DisplayGrid) ? gPad->SetGrid(true, true) : gPad->SetGrid(false, false);
  
  TheCanvas_C->Update();

  vector<TGraph *>::iterator it = WaveformGraphs.begin();
  for(; it!=WaveformGraphs.end(); it++)
    delete (*it);
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
    XTitle = "Pulse value [ADC]";
    YTitle = "Counts";
    
    XSize = YSize = 0.05;
    XOffset = 1.1;
    YOffset = 1.2;
  }
}


void AAGraphics::PlotSpectrum(TH1F *Spectrum_H)
{
  int Channel = TheSettings->SpectrumChannel;
  
  Spectrum_H->Draw("C");

  // Set spectrum graphical attributes

  Spectrum_H->SetLineColor(ChColor[Channel]);
  Spectrum_H->SetLineWidth(SpectrumWidth);

  // Set spectrum axes range and lin/log 

  double XMin = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMin;
  double XMax = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMax;
  Spectrum_H->GetXaxis()->SetRangeUser(XMin, XMax);
  
  (TheSettings->DisplayXAxisInLog) ? 
    gPad->SetLogx(true) : gPad->SetLogx(false);
  
  int AbsoluteMax = Spectrum_H->GetBinContent(Spectrum_H->GetMaximumBin());
  double YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
  double YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
  
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


  // If calibration is enabled the draw a vertical line corresponding
  // to the current pulse value selected by the triple slider pointer
  if(TheSettings->SpectrumCalibrationEnable){
    double PulseValue = TheSettings->SpectrumMaxBin *
      TheSettings->HorizontalSliderPtr;
    
    SpectrumCalibration_L->DrawLine(PulseValue,
				    YMin,
				    PulseValue,
				    YMax);
  }
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




      /*

	    // At minimum, a single channel's waveform is graphed. The
	    // "lowest enabled channel", ie, the channel closest to 0
	    // that is plotted, must set the graphical attributes of the
	    // plot, including defining the X and Y axies; subsequent
	    // channel waveform graphs will then be plotted on top.
	    
	    if(ch==LowestEnabledChannel){
	      DGScopeWaveform_G[ch]->SetLineWidth(2);
	      DGScopeWaveform_G[ch]->SetLineColor(ch+1);
	      DGScopeWaveform_G[ch]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
	      DGScopeWaveform_G[ch]->GetXaxis()->SetTitle(DGScopeDisplayXTitle_TEL->GetEntry()->GetText());
	      DGScopeWaveform_G[ch]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
	      DGScopeWaveform_G[ch]->GetXaxis()->CenterTitle();
	      DGScopeWaveform_G[ch]->GetXaxis()->SetRangeUser(xMin, xMax);
	      DGScopeWaveform_G[ch]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
	      DGScopeWaveform_G[ch]->GetYaxis()->CenterTitle();
	      DGScopeWaveform_G[ch]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
	      DGScopeWaveform_G[ch]->GetYaxis()->SetRangeUser(yMin,yMax);
	      DGScopeWaveform_G[ch]->Draw("AL");
	    }
	    else{
	      DGScopeWaveform_G[ch]->SetLineWidth(2);
	      DGScopeWaveform_G[ch]->SetLineColor(ch+1);
	      DGScopeWaveform_G[ch]->Draw("L");
	    }
*/
