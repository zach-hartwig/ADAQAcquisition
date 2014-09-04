#include "AAGraphics.hh"

AAGraphics::AAGraphics()
{;}


AAGraphics::~AAGraphics()
{;}


  /*
  // Create string array used to assign labels for each channel in the
  // DGScopeWaveform_Leg ROOT legend object
  string DGScopeWaveformTitle[8] = {"Ch 0", "Ch 1", "Ch 2", "Ch 3", 
				    "Ch 4", "Ch 5", "Ch 6", "Ch 7"};
  
  // Create a ROOT legend for the waveform graph
  DGScopeWaveform_Leg = new TLegend(0.91, 0.5, 0.99, 0.95);
  
  // For each channel on the digitizer, create the appropriate label
  // and symbol in the ROOT legend using a dummy TGraph object to set
  // the line attributes. Also, initialize the TH1F objects
  // representing the pulse heigh spectrum for each channel
  for(int i=0; i<NumDataChannels; i++){
    TGraph *Dummy_G = new TGraph();
    Dummy_G->SetLineColor(i+1);
    Dummy_G->SetLineWidth(4);

    assert(i<9);

    DGScopeWaveform_Leg->AddEntry(Dummy_G, DGScopeWaveformTitle[i].c_str(), "L");
    DGScopeWaveform_Leg->SetFillColor(18);
    DGScopeWaveform_Leg->SetTextSize(0.04);

    DGScopeChannelTrigger_L[i] = new TLine;
    DGScopeChannelTrigger_L[i]->SetLineColor(i+1);
    DGScopeChannelTrigger_L[i]->SetLineWidth(2);
    DGScopeChannelTrigger_L[i]->SetLineStyle(7);
    
    DGScopeBaselineCalcRegion_B[i] = new TBox;
    DGScopeBaselineCalcRegion_B[i]->SetFillColor(i+1);
    DGScopeBaselineCalcRegion_B[i]->SetFillStyle(3001);
  }

  DGScopeSpectrumCalibration_L = new TLine;
  DGScopeSpectrumCalibration_L->SetLineColor(kRed);
  DGScopeSpectrumCalibration_L->SetLineWidth(2);
  DGScopeSpectrumCalibration_L->SetLineStyle(7);
  

  // Create a dummy TLine object to add a single entry to thewaveform
  // graph legend representing the trigger lines
  TLine *Dummy_Line = new TLine();
  Dummy_Line->SetLineColor(4);
  Dummy_Line->SetLineStyle(2);
  Dummy_Line->SetLineWidth(4);
  DGScopeWaveform_Leg->AddEntry(Dummy_Line, "Trig", "L");
  
  for(int ch=0; ch<NumDataChannels; ch++){
    DGScopeWaveform_G[ch] = new TGraph;
    DGScopeSpectrum_H[ch] = new TH1F;

    UseCalibrationManager.push_back(false);
    CalibrationManager.push_back(new TGraph);
    ADAQChannelCalibrationData Init;
    CalibrationData.push_back(Init);
  }
  */


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
