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
*/
