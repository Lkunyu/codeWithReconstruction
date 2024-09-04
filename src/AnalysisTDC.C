#include "AnalysisTDC.h"

AnalysisTDC::AnalysisTDC(TString fRawName, TString fAnaName, int f_force)
{
  rawName = fRawName;
  anaName = fAnaName;
  force = f_force;
}

AnalysisTDC::~AnalysisTDC()
{
}

bool AnalysisTDC::Cut()
{
  bool iscut = true;
  if (board == 0)
    iscut = iscut && TOA > -3.1e6 && TOA < -2.72e6; // TOA>-1.762e6 && TOA<-1.727e6;
  if (board == 0)
    iscut = iscut && TOT > 400 && TOT < T0_TOT_max[channel] + 50;

  if (board >= 1)
    iscut = iscut && TOA > -2.16e6 && TOA < -1.95e6;
  if (board >= 1)
    iscut = iscut && TOT > 500;
  // if(board >= 1) iscut = iscut && TOT>500 && TOT<DTOF_TOT_max[(board-1)*32+channel]+50;
  return iscut;
}

bool AnalysisTDC::Init(TString caliparsName)
{
  if (!getCaliPars(caliparsName))
  {
    cout << " ERROR! Calibration pars not exist!" << endl;
    return false;
  }

  frawFile = new TFile(rawName, "read");
  if (!frawFile->IsOpen())
  {
    cout << " ERROR! " << rawName << " cant open!" << endl;
    return false;
  }
  cout << " --> Analyzing TDC data files from " << rawName << endl;
  frawTree = (TTree *)frawFile->Get("DtofRaw");
  if (frawTree == 0)
  {
    cout << " ERROR! DtofRaw Tree not exist in " << rawName << endl;
    return false;
  }
  frawTree->SetBranchAddress("event", &event, &b_event);
  frawTree->SetBranchAddress("board", &board, &b_board);
  frawTree->SetBranchAddress("channel", &channel, &b_channel);
  frawTree->SetBranchAddress("TOA", &TOA, &b_TOA);
  frawTree->SetBranchAddress("TOT", &TOT, &b_TOT);
  frawTree->SetBranchAddress("tempFPGA", &tempFPGA, &b_tempFPGA);
  frawTree->SetBranchAddress("tempSFP", &tempSFP, &b_tempSFP);

  if (force != 1)
  {
    FileStat_t fStat;
    gSystem->GetPathInfo(anaName, fStat);
    if (fStat.fSize != 0)
    {
      cout << " ERROR! " << anaName << " is exist!" << endl;
      return false;
    }
  }

  fanaFile = new TFile(anaName, "recreate");
  if (!fanaFile->IsOpen())
  {
    cout << "ERROR! " << anaName << " cant open!" << endl;
    return false;
  }
  fanaTree = new TTree("DtofData", "DtofData");
  fanaTree->Branch("event", &event, "event/I");
  fanaTree->Branch("T0_Npe", &T0_Npe, "T0_Npe/I");
  fanaTree->Branch("T0_TOT", T0_TOT, "T0_TOT[4]/D");
  fanaTree->Branch("T0_TOA", T0_TOA, "T0_TOA[4]/D");
  fanaTree->Branch("T0_temp", T0_temp, "T0_temp[4]/D");
  fanaTree->Branch("DTOF_Npe", &DTOF_Npe, "DTOF_Npe/I");
  fanaTree->Branch("DTOF_TOT", DTOF_TOT, "DTOF_TOT[672]/D");
  fanaTree->Branch("DTOF_TOA", DTOF_TOA, "DTOF_TOA[672]/D");
  fanaTree->Branch("DTOF_temp", DTOF_temp, "DTOF_temp[672]/D");

  return true;
}

void AnalysisTDC::Loop()
{
  ofstream op = LogFile(anaName);
  TCanvas *cTmp = new TCanvas("tmptdcana", "tmptdcana");
  TH1F *hT0tot[4];
  TH1F *hT0temp[4];
  for (int i = 0; i < 4; i++)
  {
    hT0tot[i] = new TH1F(Form("T0%d_TOT", i), Form("T0%d_TOT", i), 400, 200, 1500);
    hT0tot[i]->GetXaxis()->SetTitle("TOT [ps]");
    hT0tot[i]->GetYaxis()->SetTitle("Counts");
    hT0temp[i] = new TH1F(Form("T0%d_FPGAtemp", i), Form("T0%d_FPGAtemp", i), 400, 25, 45);
    hT0temp[i]->GetXaxis()->SetTitle("FPGA temperature [Celsius]");
    hT0temp[i]->GetYaxis()->SetTitle("Counts");
  }
  TH1I *hT0Eff = new TH1I("T0_Eff", "T0_Eff", 5, 0, 5);
  hT0Eff->GetXaxis()->SetTitle("Number of fired PMTs");
  hT0Eff->GetYaxis()->SetTitle("Counts");
  hT0Eff->SetLineWidth(2);
  TH1I *hT0PMTEff = new TH1I("T0_PMT_Eff", "T0_PMT_Eff", 4, 0, 4);
  hT0PMTEff->GetXaxis()->SetTitle("Fired PMT ID");
  hT0PMTEff->GetYaxis()->SetTitle("Counts");
  hT0PMTEff->SetLineWidth(2);

  TH1F *hDTOFtot[672];
  for (int i = 0; i < 672; i++)
  {
    hDTOFtot[i] = new TH1F(Form("DTOF_PMT%02dch%02d_TOT", i / 16, i % 16), Form("DTOF_PMT%02dch%02d_TOT", i / 16, i % 16), 400, 200, 1500);
    hDTOFtot[i]->GetXaxis()->SetTitle("TOT [ps]");
    hDTOFtot[i]->GetYaxis()->SetTitle("Counts");
  }
  TH1F *hDTOFtemp[21];
  for (int i = 0; i < 21; i++)
  {
    hDTOFtemp[i] = new TH1F(Form("DTOF_board%d_FPGAtemp", i + 1), Form("DTOF_board%d_FPGAtemp", i + 1), 400, 30, 50);
    hDTOFtemp[i]->GetXaxis()->SetTitle("FPGA temperature [Celsius]");
    hDTOFtemp[i]->GetYaxis()->SetTitle("Counts");
  }
  TH1I *hDTOFnpe = new TH1I("DTOF_Npe", "DTOF_Npe", 150, 0, 150);
  hDTOFnpe->GetXaxis()->SetTitle("Number of photoelectrons");
  hDTOFnpe->GetYaxis()->SetTitle("Counts");
  hDTOFnpe->SetLineWidth(2);
  TH1I *hDTOFchEff = new TH1I("DTOF_ch_Eff", "DTOF_ch_Eff", 672, 0, 672);
  hDTOFchEff->GetXaxis()->SetTitle("Fired channel ID");
  hDTOFchEff->GetYaxis()->SetTitle("Counts");
  hDTOFchEff->SetLineWidth(2);
  TH2I *hDTOFPMTEff = new TH2I("DTOF_PMT_Eff", "DTOF_PMT_Eff", 42, 0, 42, 16, 0, 16);
  hDTOFPMTEff->GetXaxis()->SetTitle("PMT ID");
  hDTOFPMTEff->GetYaxis()->SetTitle("Number of photoelectrons");

  /////////// begin to loop
  vector<int> event_vec, board_vec, channel_vec;
  vector<double> TOA_vec, TOT_vec, tempFPGA_vec, tempSFP_vec;
  for (int i = 0; i < frawTree->GetEntries(); i++)
  {
    frawTree->GetEntry(i);
    if (board == 0 && channel == 101)
    {
      tempMaster1 = tempFPGA;
      tempMaster2 = tempSFP;
      continue;
    }
    if (tempMaster1 != 0 && tempMaster2 != 0)
    {
        calibration();
    }

    event_vec.push_back(event);
    board_vec.push_back(board);
    channel_vec.push_back(channel);
    TOA_vec.push_back(TOA);
    TOT_vec.push_back(TOT);
    tempFPGA_vec.push_back(tempFPGA);
    tempSFP_vec.push_back(tempSFP);
  }
  frawFile->Close();
  cout << " --> read data from " << rawName << " successfully !" << endl;
  op << " --> read data from " << rawName << " successfully !" << endl;

  int start = event_vec[0];
  if (start == 0)
    start = event_vec[1];
  int end = event_vec[event_vec.size() - 1];
  int totaltriggers = end - start;
  cout << "start " << 0 << " end " << event_vec[event_vec.size() - 1] << endl;
  cout << " --> total events: " << totaltriggers << ", start from " << start << " to " << end << endl;
  op << " --> total events: " << totaltriggers << ", start from " << start << " to " << end << endl;

  for (int i = 0; i < event_vec.size(); i++)
  {
    DrawProcessbar(i, event_vec.size());
    if (board_vec[i] != 0 || channel_vec[i] != 100)
      continue;
    event = event_vec[i];
    triggertime = TOA_vec[i];                                                // cout<<"event "<<event<<" triggerTime "<<triggertime<<endl;
    triggertimelow = long(triggertime / 1562.5) % long(pow(2, 30)) * 1562.5; // cout<<"triggertime "<<triggertime<<" "<<triggertimelow<<endl;
    InitData();

    int pos1 = i, pos2 = i;
    for (int j = i; j >= 0; j--)
      if (event_vec[j] == event)
        pos1 = j;
      else
        break;
    for (int j = i; j < event_vec.size(); j++)
      if (event_vec[j] == event)
        pos2 = j;
      else
        break;

    for (int j = pos1; j <= pos2; j++)
    {
      if (event_vec[j] != event)
        continue;
      board = board_vec[j];
      channel = channel_vec[j];
      if (board >= 16)
        TOA = TOA_vec[j] - triggertimelow;
      else
        TOA = TOA_vec[j] - triggertime;
      TOT = TOT_vec[j];
      tempFPGA = tempFPGA_vec[j];
      if (channel > 32)
        continue;
      if (!Cut())
        continue;

      if (board == 0 && channel >= 1)
        channel = channel - 1;
      if (board >= 1)
        channel = (board - 1) * 32 + channel;

      if (board == 0)
      {
        if (T0_TOT[channel] == 0)
        {
          T0_Npe++;
          T0_TOT[channel] = TOT;
          T0_TOA[channel] = TOA;
          T0_temp[channel] = tempFPGA;

          hT0tot[channel]->Fill(TOT);
          hT0temp[channel]->Fill(tempFPGA);
          hT0PMTEff->Fill(channel);
        }
        else if (TOA < T0_TOA[channel])
        {
          T0_TOT[channel] = TOT;
          T0_TOA[channel] = TOA;
          T0_temp[channel] = tempFPGA;
        }
      }
      if (board >= 1)
      {
        if (DTOF_TOT[channel] == 0)
        {
          DTOF_Npe++;
          DTOF_TOT[channel] = TOT;
          DTOF_TOA[channel] = TOA; // cout<<"eventID "<<event<<" TOA "<<TOA<<" TOT "<<TOT<<" channel "<<channel<<endl;
          DTOF_temp[channel] = tempFPGA;

          hDTOFtot[channel]->Fill(TOT);
          hDTOFtemp[channel / 32]->Fill(tempFPGA);
          hDTOFchEff->Fill(channel);
        }
        else if (TOA < DTOF_TOA[channel])
        {
          DTOF_TOT[channel] = TOT;
          DTOF_TOA[channel] = TOA; // cout<<"eventID "<<event<<" TOA "<<TOA<<" TOT "<<TOT<<" channel "<<channel<<endl;
          DTOF_temp[channel] = tempFPGA;
        }
      }
    } // for each event
    hT0Eff->Fill(T0_Npe);
    hDTOFnpe->Fill(DTOF_Npe);
    for (int j = 0; j < 42; j++)
    {
      int npe = 0;
      for (int k = 0; k < 16; k++)
        if (DTOF_TOA[j * 16 + k] != 0)
          npe++;
      hDTOFPMTEff->Fill(j, npe);
    }
    fanaTree->Fill();
  } // TDC data loop

  fanaFile->WriteTObject(fanaTree);

  // Draw hit information
  TLegend *leg;
  TLatex *la;
  int maximum_counts = 0;
  TString latexbuff;

  // Draw T0 hit information
  TCanvas *cT0 = new TCanvas("T0hit", "T0hit");
  cT0->Divide(2, 2);

  cT0->cd(1);
  leg = new TLegend(0.2, 0.5, 0.3, 0.85);
  maximum_counts = 0;
  for (int i = 0; i < 4; i++)
    if (maximum_counts < hT0tot[i]->GetMaximum())
      maximum_counts = hT0tot[i]->GetMaximum();
  for (int i = 0; i < 4; i++)
  {
    hT0tot[i]->SetLineColor(color[i]);
    hT0tot[i]->GetYaxis()->SetRangeUser(0, maximum_counts * 1.3);
    if (i == 0)
      hT0tot[i]->Draw();
    else
      hT0tot[i]->Draw("same");
    leg->AddEntry(hT0tot[i], Form("T0%d", i), "lpf");
  }
  leg->Draw("same");

  cT0->cd(2);
  leg = new TLegend(0.2, 0.5, 0.3, 0.85);
  maximum_counts = 0;
  for (int i = 0; i < 4; i++)
    if (maximum_counts < hT0temp[i]->GetMaximum())
      maximum_counts = hT0temp[i]->GetMaximum();
  for (int i = 0; i < 4; i++)
  {
    hT0temp[i]->SetLineColor(color[i]);
    hT0temp[i]->GetYaxis()->SetRangeUser(0, maximum_counts * 1.3);
    if (i == 0)
      hT0temp[i]->Draw();
    else
      hT0temp[i]->Draw("same");
    leg->AddEntry(hT0temp[i], Form("T0%d", i), "lpf");
    latexbuff = Form("T0%d board temperature=%.1f degrees", i, hT0temp[i]->GetMean());
    cout << " --> " << latexbuff << endl;
    op << " --> " << latexbuff << endl;
  }
  leg->Draw("same");

  cT0->cd(3);
  hT0Eff->GetYaxis()->SetRangeUser(0, hT0Eff->GetMaximum() * 1.1);
  hT0Eff->Draw();
  double T0Eff[5];
  for (int i = 0; i < 5; i++)
  {
    T0Eff[i] = hT0Eff->GetBinContent(hT0Eff->FindBin(i)) / totaltriggers;
    latexbuff = Form("NPMT=%d, Ratio=%.2f%%", i, T0Eff[i] * 100);
    cout << " --> T0 " << latexbuff << endl;
    op << " --> T0 " << latexbuff << endl;
    la = CreatLatex(latexbuff, 0.25, 0.7 - 0.07 * i, 42);
    la->Draw("same");
  }

  cT0->cd(4);
  hT0PMTEff->GetYaxis()->SetRangeUser(0, hT0PMTEff->GetMaximum() * 1.2);
  hT0PMTEff->Draw();
  double T0PMTEff[4];
  for (int i = 0; i < 4; i++)
  {
    T0PMTEff[i] = hT0PMTEff->GetBinContent(hT0PMTEff->FindBin(i)) / totaltriggers;
    latexbuff = Form("PMTID=%d, Ratio=%.2f%%", i, T0PMTEff[i] * 100);
    cout << " --> T0 " << latexbuff << endl;
    op << " --> T0 " << latexbuff << endl;
    la = CreatLatex(latexbuff, 0.25, 0.7 - 0.07 * i, 42);
    la->Draw("same");
  }

  fanaFile->WriteTObject(cT0);

  // Draw DTOF hit information
  TCanvas *cDTOFtot = new TCanvas("DTOFtot", "DTOFtot");
  cDTOFtot->Divide(7, 6);
  for (int i = 0; i < 42; i++)
  {
    cDTOFtot->cd(i + 1);
    leg = new TLegend(0.65, 0.13, 0.87, 0.87);
    maximum_counts = 0;
    for (int j = 0; j < 16; j++)
      if (maximum_counts < hDTOFtot[i * 16 + j]->GetMaximum())
        maximum_counts = hDTOFtot[i * 16 + j]->GetMaximum();
    for (int j = 0; j < 16; j++)
    {
      hDTOFtot[i * 16 + j]->SetLineColor(color[j]);
      hDTOFtot[i * 16 + j]->GetYaxis()->SetRangeUser(0, maximum_counts * 1.3);
      if (j == 0)
        hDTOFtot[i * 16 + j]->Draw();
      else
        hDTOFtot[i * 16 + j]->Draw("same");
      leg->AddEntry(hDTOFtot[i * 16 + j], Form("PMT%02dch%02d", i, j), "lpf");
    }
    leg->Draw("same");
  }
  fanaFile->WriteTObject(cDTOFtot);

  TCanvas *cDTOF = new TCanvas("DTOFhit", "DTOFhit");
  cDTOF->Divide(2, 2);

  cDTOF->cd(1);
  hDTOFPMTEff->Draw("COLZ");

  cDTOF->cd(2);
  leg = new TLegend(0.15, 0.15, 0.35, 0.85);
  maximum_counts = 0;
  for (int i = 0; i < 21; i++)
    if (maximum_counts < hDTOFtemp[i]->GetMaximum())
      maximum_counts = hDTOFtemp[i]->GetMaximum();
  for (int i = 0; i < 21; i++)
  {
    hDTOFtemp[i]->SetLineColor(color[i]);
    hDTOFtemp[i]->GetYaxis()->SetRangeUser(0, maximum_counts * 1.3);
    if (i == 0)
      hDTOFtemp[i]->Draw();
    else
      hDTOFtemp[i]->Draw("same");
    leg->AddEntry(hDTOFtemp[i], Form("DTOF_board%d", i + 1), "lpf");
    latexbuff = Form("board%d temperature=%.1f degrees", i, hDTOFtemp[i]->GetMean());
    cout << " --> DTOF " << latexbuff << endl;
    op << " --> DTOF " << latexbuff << endl;
  }
  leg->Draw("same");

  cDTOF->cd(3);
  hDTOFchEff->Draw();

  cDTOF->cd(4);
  hDTOFnpe->Draw();
  latexbuff = Form("#color[2]{N_{pe}=%.1f}", hDTOFnpe->GetMean());
  la = CreatLatex(latexbuff, 0.6, 0.35);
  la->Draw("same");
  latexbuff = Form("Npe=%.1f", hDTOFnpe->GetMean());
  cout << " --> DTOF " << latexbuff << endl;
  op << " --> DTOF " << latexbuff << endl;

  fanaFile->WriteTObject(cDTOF);

  fanaFile->Close();
  cout << " --> Analyzed results has been stored to " << anaName << endl;
  op << " --> Analyzed results has been stored to " << anaName << endl;
  op.close();
}

void AnalysisTDC::InitData()
{
  T0_Npe = 0;
  memset(T0_TOT, 0, sizeof(T0_TOT));
  memset(T0_TOA, 0, sizeof(T0_TOA));
  memset(T0_temp, 0, sizeof(T0_temp));
  DTOF_Npe = 0;
  memset(DTOF_TOT, 0, sizeof(DTOF_TOT));
  memset(DTOF_TOA, 0, sizeof(DTOF_TOA));
  memset(DTOF_temp, 0, sizeof(DTOF_temp));
}

bool AnalysisTDC::getCaliPars(TString filename)
{
  ifstream pars_file;
  pars_file.open(filename.Data());
  string line, par;
  vector<double> pars;
  if (pars_file.is_open())
  {
    // read the pars name
    getline(pars_file, line);
    // read master temp pars
    for (int i = 0; i < 21; i++)
    {
      getline(pars_file, line);
      istringstream line_stream(line);
      while (line_stream)
      {
        if (!getline(line_stream, par, ',') || par == "")
          break;
        pars.push_back(stod(par));
      }
      if (pars.size() != 2)
      {
        cout << " --> " << filename << ", master temp pars error !!!" << endl;
        pars_file.close();
        return false;
      }
      master_temp_par[int(pars[0] - 1)] = pars[1];
      pars.clear();
    }
    // read the pars name
    getline(pars_file, line);
    // read T0 calibration pars
    for (int i = 0; i < 5; i++)
    {
      getline(pars_file, line);
      istringstream line_stream(line);
      while (line_stream)
      {
        if (!getline(line_stream, par, ',') || par == "")
          break;
        pars.push_back(stod(par));
      }
      if (pars.size() != 7)
      {
        cout << " --> " << filename << ", T0 calibration pars error !!!" << endl;
        pars_file.close();
        return false;
      }
      T0_TOT_max[int(pars[1])] = pars[2];
      T0_TOT_mean[int(pars[1])] = pars[3];
      T0_TOT_par[int(pars[1])] = pars[4];
      T0_temp_par[int(pars[1])] = pars[5];
      T0_offset[int(pars[1])] = pars[6];
      pars.clear();
    }
    // read DTOF calibration pars
    for (int i = 0; i < 672; i++)
    {
      getline(pars_file, line);
      istringstream line_stream(line);
      while (line_stream)
      {
        if (!getline(line_stream, par, ',') || par == "")
          break;
        pars.push_back(stod(par));
      }
      if (pars.size() != 7)
      {
        cout << " --> " << filename << ", DTOF calibration pars error !!!" << endl;
        pars_file.close();
        return false;
      }
      DTOF_TOT_max[int((pars[0] - 1) * 32 + pars[1])] = pars[2];
      DTOF_TOT_mean[int((pars[0] - 1) * 32 + pars[1])] = pars[3];
      DTOF_TOT_par[int((pars[0] - 1) * 32 + pars[1])] = pars[4];
      DTOF_temp_par[int((pars[0] - 1) * 32 + pars[1])] = pars[5];
      DTOF_offset[int((pars[0] - 1) * 32 + pars[1])] = pars[6];
      pars.clear();
    }
  }
  else
  {
    cout << " --> File " << filename << " open error!!!" << endl;
    pars_file.close();
    return false;
  }
  pars_file.close();

  /*for(int i=0; i<21; i++)
    cout<<i<<" "<<master_temp_par[i]<<endl;
  for(int i=0; i<5; i++)
    cout<<i<<" "<<T0_TOT_max[i]<<" "<<T0_TOT_mean[i]<<" "
      <<T0_TOT_par[i]<<" "<<T0_temp_par[i]<<" "<<T0_offset[i]<<endl;
  for(int i=0; i<672; i++)
    cout<<i<<" "<<DTOF_TOT_max[i]<<" "<<DTOF_TOT_mean[i]<<" "
      <<DTOF_TOT_par[i]<<" "<<DTOF_temp_par[i]<<" "<<DTOF_offset[i]<<endl;*/
  return true;
}

void AnalysisTDC::calibration()
{
  // TOTcalibration();
  // tempcalibration();
  // mastertempcalibration();

  if (board == 0 && channel <= 4)
    TOA = TOA - T0_offset[channel];
  if (board >= 1)
  {
    if ((board - 1) * 32 + channel >= 448)//DTOF channels
    {
      TOTcalibration();
      tempcalibration();
      mastertempcalibration();
    }
    int i = (board - 1) * 32 + channel;
    TOA = TOA - DTOF_offset[i];
  }
}


void AnalysisTDC::TOTcalibration()
{
  if (board == 0 && channel <= 4)
    TOA = TOA - T0_TOT_par[channel] * (TOT - T0_TOT_mean[channel]);
  if (board >= 1)
  {
    int i = (board - 1) * 32 + channel;
    TOA = TOA - DTOF_TOT_par[i] * (TOT - DTOF_TOT_mean[i]);
  }
}

void AnalysisTDC::tempcalibration()
{
  if (board == 0 && channel <= 4)
    TOA = TOA - T0_temp_par[channel] * (tempFPGA - 28); // 28 Celsius
  if (board >= 1)
  {
    int i = (board - 1) * 32 + channel;
    TOA = TOA - DTOF_temp_par[i] * (tempFPGA - 36); // 36 celsius
  }
}

void AnalysisTDC::mastertempcalibration()
{
  // board 0-10 group1, baord 11-21 group2, two borads lost
  if (board >= 1 && board <= 10)
    TOA = TOA; // - master_temp_par[board-1]*(tempMaster1-49); // 49 celsius
  if (board >= 11)
    TOA = TOA; //- master_temp_par[board-1]*(tempMaster2-46); // 49 celsius
}
