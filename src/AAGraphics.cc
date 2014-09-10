#include <TGraph.h>
#include <TStyle.h>
#include <TFrame.h>

#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

#include <iostream>
#include <sstream>

#include "AAGraphics.hh"
#include "AAVMEManager.hh"
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

    float XMin, XMax, YMin, YMax;

    XMin = WaveformLength * TheSettings->HorizontalSliderMin;
    XMax = WaveformLength * TheSettings->HorizontalSliderMax;
    Waveform_G->GetXaxis()->SetRangeUser(XMin, XMax);
    
    int AbsoluteMax = AAVMEManager::GetInstance()->GetDGManager()->GetMaxBit();
    YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
    YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
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


void AAGraphics::PlotSpectrum(vector<TH1F *> &Spectrum_H)
{
  int Channel = TheSettings->SpectrumChannel;

  Spectrum_H[Channel]->Draw("C");

  Spectrum_H[Channel]->SetLineColor(ChColor[Channel]);
  Spectrum_H[Channel]->SetLineWidth(SpectrumWidth);


  float XMin, XMax, YMin, YMax;

  XMin = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMin;
  XMax = TheSettings->SpectrumMaxBin * TheSettings->HorizontalSliderMax;
  Spectrum_H[Channel]->GetXaxis()->SetRangeUser(XMin, XMax);
  
  int AbsoluteMax = Spectrum_H[Channel]->GetMaximum() * 1.05;

  YMin = AbsoluteMax * TheSettings->VerticalSliderMin;
  YMax = AbsoluteMax * TheSettings->VerticalSliderMax;
  //  Spectrum_H[Channel]->SetMinimum(YMin);
  //  Spectrum_H[Channel]->SetMaximum(YMax);

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


void  AAGraphics::PlotCalibration()
{}




// Method that enables the user to force the drawing of the pulse
// spectrum histogram for the specified channel. This can be used
// while acquiring data (most importantly when the user is acquiring
// data in high-throughput mode where nothing is plotted by default,
// he/she can force plotting to evaluate the progress/validity of the
// spectrum without slowing down acquisition) or after the acquisition
// is complete and turned off (setting titles, positions, etc) for
// pretty output to file.
/*
void AAInterface::ForceSpectrumDrawing()
{
  int ChannelToHistogram = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
  
  float xMin, xMax, yMin, yMax;
  int maxBin = DGScopeSpectrumMaxBin_NEL->GetEntry()->GetIntNumber();
  
  DGScopeHorizontalScale_THS->GetPosition(&xMin, &xMax);
  DGScopeVerticalScale_DVS->GetPosition(&yMin, &yMax);
  
  xMin *= maxBin;
  xMax *= maxBin;
  
  yMin *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin());
  yMax *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin())*1.1;
  
  // Enable the X and Y axes to be plotted on a log. scale
  if(DGScopeSpectrumXAxisLog_CB->IsDown())
    DGScope_EC->GetCanvas()->SetLogx(true);
  else
    DGScope_EC->GetCanvas()->SetLogx(false);
  
  if(DGScopeSpectrumYAxisLog_CB->IsDown()){
    DGScope_EC->GetCanvas()->SetLogy();
    yMin += 1;
  }
  else
    DGScope_EC->GetCanvas()->SetLogy(false);
  
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitle(DGScopeDisplayXTitle_TEL->GetEntry()->GetText());
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->CenterTitle();
  DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetRangeUser(xMin,xMax);
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->CenterTitle();
  DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetRangeUser(yMin,yMax);
  DGScopeSpectrum_H[ChannelToHistogram]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
  DGScopeSpectrum_H[ChannelToHistogram]->Draw("L");

  DGScope_EC->GetCanvas()->Update();
}



	    DGScope_EC->GetCanvas()->SetLogx(false);
	    DGScope_EC->GetCanvas()->SetLogy(false);
	    
	    // Ensure to free previous memory allocated to the TGraphs
	    // to prevent fairly massive memory leakage
	    if(DGScopeWaveform_G[ch]) delete DGScopeWaveform_G[ch];
	    DGScopeWaveform_G[ch] = new TGraph(RecordLength, Time_graph, Voltage_graph);
	    
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
	      
	      if(DrawLegend)
		DGScopeWaveform_Leg->Draw();
	    }
	    else{
	      DGScopeWaveform_G[ch]->SetLineWidth(2);
	      DGScopeWaveform_G[ch]->SetLineColor(ch+1);
	      DGScopeWaveform_G[ch]->Draw("L");
	    }
	    
	    // Draw a horizontal dotted line of the same color as the
	    // channel waveform representing the channel trigger
	    // threshold. Ensure accounting for channel vertical offset
	    VertPosOffset = DGScopeVerticalPosition_NEL[ch]->GetEntry()->GetIntNumber();
	    ChTrigThr = (DGScopeChTriggerThreshold_NEL[ch]->GetEntry()->GetIntNumber() + VertPosOffset) * ConvertVoltageToGraphUnits;
	    
	    DGScopeChannelTrigger_L[ch]->DrawLine(xMin, ChTrigThr, xMax, ChTrigThr);
	    
	    // Draw a shaded box region to represent the area of the
	    // waveform being used to calculate the current baseline for
	    // each digitized waveform
	    BaseCalcMin = BaselineCalcMin[ch]*ConvertTimeToGraphUnits;
	    BaseCalcMax = BaselineCalcMax[ch]*ConvertTimeToGraphUnits;
	    BaseCalcResult = (BaselineCalcResult[ch] + VertPosOffset) * ConvertVoltageToGraphUnits;

	    DGScopeBaselineCalcRegion_B[ch]->DrawBox(BaseCalcMin, (BaseCalcResult-100), BaseCalcMax, (BaseCalcResult+100));
	  }


	  // Determine the channel desired for histogramming into a pulse height spectrum
	  ChannelToHistogram = DGScopeSpectrumChannel_CBL->GetComboBox()->GetSelected();
	  
	  // Update the spectrum when specified by the user
	  if(int(DGScopeSpectrum_H[ChannelToHistogram]->GetEntries())%SpectrumRefreshRate==0){
	    
	    // Need to get the raw positions of the sliders again at
	    // present since variables are transformed automatically
	    // for waveform plotting at top of acquisition loop
	    DGScopeHorizontalScale_THS->GetPosition(&xMin, &xMax);
	    DGScopeVerticalScale_DVS->GetPosition(&yMin, &yMax);
	    
	    xMin *= maxBin;
	    xMax *= maxBin;

	    yMin *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin());
	    yMax *= DGScopeSpectrum_H[ChannelToHistogram]->GetBinContent(DGScopeSpectrum_H[ChannelToHistogram]->GetMaximumBin())*1.1;
	    
	    // Enable the X and Y axes to be plotted on a log. scale
	    if(DGScopeSpectrumXAxisLog_CB->IsDown())
	      DGScope_EC->GetCanvas()->SetLogx(true);
	    else
	      DGScope_EC->GetCanvas()->SetLogx(false);
	    
	    if(DGScopeSpectrumYAxisLog_CB->IsDown()){
	      DGScope_EC->GetCanvas()->SetLogy();
	      yMin += 1;
	    }
	    else
	      DGScope_EC->GetCanvas()->SetLogy(false);

	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitle(DGScopeDisplayXTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetTitleOffset(DGScopeDisplayXTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetXaxis()->SetRangeUser(xMin,xMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitle(DGScopeDisplayYTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetTitleOffset(DGScopeDisplayYTitleOffset_NEL->GetEntry()->GetNumber());
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->CenterTitle();
	    DGScopeSpectrum_H[ChannelToHistogram]->GetYaxis()->SetRangeUser(yMin,yMax);
	    DGScopeSpectrum_H[ChannelToHistogram]->SetTitle(DGScopeDisplayTitle_TEL->GetEntry()->GetText());
	    DGScopeSpectrum_H[ChannelToHistogram]->Draw("L");
	    
	    // If the calibration mode is enabled, then draw the third
	    // slider position from the horizontal triple slider and
	    // use its position to calculate the correct calibration
	    // factor. Update the calibration factor number entry
	    // widget with the calculated value

	    // If the user has enabled spectrum calibration mode ...
	    if(DGScopeSpectrumCalibration_CB->IsDown()){
	      
	      double LinePosX;

	      // If the user has enabled the use of the calibration
	      // slider (the "pointer" on the horizontal triple slider
	      // widget located underneath the canvas) ...
	      if(DGScopeSpectrumUseCalibrationSlider_CB->IsDown()){
		
		// Get the pointer position
		LinePosX = DGScopeHorizontalScale_THS->GetPointerPosition()*maxBin;
		
		// Set the pointer position (in correct units) to the
		// ROOT GUI widget that displays the pulse unit
		DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->SetNumber(DGScopeHorizontalScale_THS->GetPointerPosition() * maxBin);
	      }
	      
	      // ... otherwise, allow the user to set the pulse unit manually
	      else
		LinePosX = DGScopeSpectrumCalibrationPulseUnit_NEL->GetEntry()->GetNumber();
	      
	      // Draw the vertical calibration line
	      DGScopeSpectrumCalibration_L->DrawLine(LinePosX, yMin, LinePosX, yMax);
	    }
	    
	    // Update the canvas
	    DGScope_EC->GetCanvas()->Update();
	  }
	}
	




      TCanvas *Calibration_C = new TCanvas("Calibration_C","CalibrationManager TGraph",0,0,600,400);
      Calibration_C->SetLeftMargin(0.14);
      Calibration_C->SetBottomMargin(0.14);
      
      stringstream ss;
      ss << "CalibrationManager TGraph for Channel[" << CurrentChannel << "]";
      string Title = ss.str();

      CalibrationManager[CurrentChannel]->SetTitle(Title.c_str());
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitle("Pulse unit [ADC]");
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetTitleOffset(1.1);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->GetXaxis()->SetNdivisions(505);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitle("Energy");
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleSize(0.06);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetTitleOffset(1.2);
      CalibrationManager[CurrentChannel]->GetYaxis()->SetLabelSize(0.06);
      CalibrationManager[CurrentChannel]->SetMarkerSize(2);
      CalibrationManager[CurrentChannel]->SetMarkerStyle(22);
      CalibrationManager[CurrentChannel]->Draw("ALP");

      Calibration_C->Update();

      DGScope_EC->GetCanvas()->cd(0);




*/
