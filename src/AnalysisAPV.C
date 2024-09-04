#include "AnalysisAPV.h"

AnalysisAPV::AnalysisAPV(TString fRawName, TString fAnaName, int f_force)
{
  rawName = fRawName;
  anaName = fAnaName;
  force = f_force;
}

AnalysisAPV::~AnalysisAPV()
{
}

bool AnalysisAPV::Cut()
{
  return maxA > pedrms * rmscut && maxT > 0 && maxT < 20 && riseT > 0 && riseT < 15 && TOT > 0 && TOT < 20;
}

bool AnalysisAPV::Init()
{
  // Reading map
  ifstream mapfile;
  mapfile.open("pars/map_APV.txt"); // board chip channel xx strip
  int mapin;
  while (mapfile >> mapin)
  {
    for (int kk = 0; kk < 3; kk++)
      mapfile >> mapin;
    mapfile >> mapin;
    APVmap.push_back(mapin); // by order
  }

  //
  frawFile = new TFile(rawName, "read");
  if (!frawFile->IsOpen())
  {
    cout << " ERROR! " << rawName << " cant open!" << endl;
    return false;
  }
  cout << " --> Analyzing waveform files from " << rawName << endl;
  frawTree = (TTree *)frawFile->Get("TrackRaw");
  if (frawTree == 0)
  {
    cout << " ERROR! TrackRaw Tree not exist in " << rawName << endl;
    return false;
  }
  frawTree->SetBranchAddress("event", &event, &b_event);
  frawTree->SetBranchAddress("board", &board, &b_board);
  frawTree->SetBranchAddress("chip", &chip, &b_chip);
  frawTree->SetBranchAddress("channel", &channel, &b_channel); // undecoded
  frawTree->SetBranchAddress("wave", &wave, &b_wave);

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
  for (int i = 0; i < 8; i++)
  {
    fanaTree[i] = new TTree(Form("Track%s%d", XYname(i % 2).Data(), i / 2), Form("Track%s%d", XYname(i % 2).Data(), i / 2));
    fanaTree[i]->Branch("event", &event);
    fanaTree[i]->Branch("maxA", &maxA);
    fanaTree[i]->Branch("maxT", &maxT);
    fanaTree[i]->Branch("channel", &channel); // det chip channel(decoded)
    fanaTree[i]->Branch("ped", &ped);
    fanaTree[i]->Branch("pedrms", &pedrms);
    fanaTree[i]->Branch("riseT", &riseT);
    fanaTree[i]->Branch("TOT", &TOT);
  }

  return true;
}

void AnalysisAPV::Loop()
{
  ofstream op = LogFile(anaName);
  TCanvas *cTmp = new TCanvas("tmpana", "tmpana");
  // TH1F *hNoise = new TH1F("Noise", "Noise", 800, 0, 800);
  TH1I *hChannel = new TH1I("Channel", "Channel", 256, 0, 256);
  hChannel->GetXaxis()->SetTitle("channel");
  hChannel->GetYaxis()->SetTitle("Counts");
  TGraph *gEvent[8];
  TH1I *hHit[8];
  for (int i = 0; i < 8; i++)
  {
    hHit[i] = new TH1I(Form("Hit%s%d", XYname(i % 2).Data(), i / 2), Form("Hit%s%d", XYname(i % 2).Data(), i / 2), 128, 0, 128);
    hHit[i]->GetXaxis()->SetTitle("channel");
    hHit[i]->GetYaxis()->SetTitle("Counts");
  }

  /////////// begin to loop
  vector<int> eventvec[8];
  vector<int> entryvec[8];
  vector<int> valideventvec[8];
  int RecordStart = -1;
  int RecordEnd = -1;
  int NumofTriggers = 0;
  int nentries = frawTree->GetEntriesFast();
  cout << " --> start to loop" << endl;
  for (int i = 0; i < nentries; i++)
  {
    DrawProcessbar(i, nentries);
    InitData();

    frawTree->GetEntry(i);
    // hNoise->Reset();
    ped = 0;
    pedrms = 0;
    XY = IndexXY(board, chip);   //////////////
    Det = IndexDet(board, chip); //////////
    channel = IndexStrip(board, chip, channel);
    if (i == 0)
    {
      RecordStart = event;
      RecordEnd = event;
    }
    if (RecordEnd < event && event - RecordEnd < 50)
    {
      RecordEnd = event;
      NumofTriggers++;
    }

    CalWaveInfo(); // maxA,maxT,ped,pedrms

    // hNoise->Fill((double)wave->at(j)); /////////

    // double tmp_midle = //hNoise->GetMaximumBin() * hNoise->GetBinWidth(0);
    //  int status = hNoise->Fit("gaus","RQ","",tmp_midle-100,tmp_midle+100);
    //  if(status == 0)
    //{
    //   ped = hNoise->GetFunction("gaus")->GetParameter(1);
    //   pedrms = hNoise->GetFunction("gaus")->GetParameter(2);
    //  ped = //hNoise->GetMean();
    //  pedrms = //hNoise->GetStdDev();
    if (ped > 0 || ped < -600 || pedrms < 0)
    {
      // cout << " [WARNING!] Bad Fit! channel: "<<64*(Det*2+XY)+channel<< endl;
      op << " [WARNING!] Bad Fit! channel: " << channel << " Det " << Det << " XY " << XY << endl; ////////////////////
      // ped = //hNoise->GetMean();
      // pedrms = //hNoise->GetRMS();
    }
    //}
    // else
    // {
    //   //cout << " ERROR! Fit false! channel: "<<64*(Det*2+XY)+channel<< endl;
    //   op << " ERROR! Fit false! channel: "<<64*(Det*2+XY)+channel<< endl;///////////
    //   ped = hNoise->GetMean();
    //   pedrms = hNoise->GetRMS();
    // }

    // maxA = TMath::MaxElement(peakend - peakstart, &(wave->at(peakstart))) - ped;
    // maxT = TMath::LocMax(peakend - peakstart, &(wave->at(peakstart))) + peakstart;
    vector<double> waveform;
    for (int i = 0; i < (int)wave->size(); i++)
      waveform.push_back((double)wave->at(i) - ped);
    riseTandTOT(waveform);

    fanaTree[Det * 2 + XY]->Fill(); /////////////////
    hChannel->Fill(channel);        ///////////////////////////
    if (eventvec[Det * 2 + XY].size() == 0 || eventvec[Det * 2 + XY].back() != event)
    {
      eventvec[Det * 2 + XY].push_back(event);
      entryvec[Det * 2 + XY].push_back(eventvec[Det * 2 + XY].size());
    }
    if (Cut())
    {
      hHit[Det * 2 + XY]->Fill(channel);
      if (valideventvec[Det * 2 + XY].size() == 0 || valideventvec[Det * 2 + XY].back() != event)
        valideventvec[Det * 2 + XY].push_back(event);
    }
  } /// loop tree
  for (int i = 0; i < 8; i++)
    fanaFile->WriteTObject(fanaTree[i]);
  hChannel->Write();

  //// check lost ID
  cout << " --> check lost event ID " << endl;
  op << " --> check lost event ID " << endl;
  TCanvas *cEvent = new TCanvas("Event", "Event");
  cEvent->Divide(4, 2);
  for (int i = 0; i < 8; i++)
  {
    TString title = Form("Track%s%dEvent", XYname(i % 2).Data(), i / 2);
    gEvent[i] = new TGraph(eventvec[i].size(), &entryvec[i][0], &eventvec[i][0]);
    gEvent[i]->SetTitle(title);
    gEvent[i]->SetName(title);
    gEvent[i]->GetXaxis()->SetTitle("entryID");
    gEvent[i]->GetYaxis()->SetTitle("eventID");
    cEvent->cd(i + 1);
    gEvent[i]->Draw();
    int lostcounter = CalculateLostID(eventvec[i]);
    cout << " --> The total numbers of " << title << ": " << eventvec[i].size() << endl
         << " --> The lost numbers of " << title << ": " << lostcounter
         << ", lostratio: " << (double)lostcounter / eventvec[i].size() << endl;
    op << " --> The total numbers of " << title << ": " << eventvec[i].size() << endl
       << " --> The lost numbers of " << title << ": " << lostcounter
       << ", lostratio: " << (double)lostcounter / eventvec[i].size() << endl;
  }
  cEvent->Write();

  //// check eff
  cout << " --> check MM efficiency " << endl;
  op << " --> check MM efficiency " << endl;
  TH1I *hMMeff[3];
  TH1I *hTrackereff[4];
  for (int i = 0; i < 3; i++)
  {
    hMMeff[i] = new TH1I(Form("MM%seff", XYname(i).Data()), Form("MM%seff", XYname(i).Data()), 4, 0, 4);
    hMMeff[i]->GetXaxis()->SetTitle("ID of MM");
    hMMeff[i]->GetYaxis()->SetTitle("Counts");
    hTrackereff[i] = new TH1I(Form("Tracker%seff", XYname(i).Data()), Form("MM%seff", XYname(i).Data()), 5, 0, 5);
    hTrackereff[i]->GetXaxis()->SetTitle("Number of fired MMs");
    hTrackereff[i]->GetYaxis()->SetTitle("Counts");
  }
  hTrackereff[3] = new TH1I("TrackerXorYeff", "TrackerXorYeff", 9, 0, 9);
  hTrackereff[3]->GetXaxis()->SetTitle("Number of fired Chips");
  hTrackereff[3]->GetYaxis()->SetTitle("Counts");
  int matchedycounter = 0;
  int matchedxcounter = 0;
  int matchedandcounter = 0;
  int matchedtotalcounter = 0;
  vector<int> matchedyvec;
  vector<int> matchedxvec;
  vector<int> matchedandvec;
  cout << " --> start to choose entries " << endl;
  for (int s = RecordStart; s < RecordEnd; s++)
  {
    DrawProcessbar(s, RecordEnd - RecordStart);
    matchedxvec.clear();
    matchedyvec.clear();
    matchedandvec.clear();
    bool matchedx = 0;
    bool matchedy = 0;
    for (int i = 0; i < 4; i++)
    {
      matchedx = IsMatched(s, valideventvec[i * 2]);
      matchedy = IsMatched(s, valideventvec[i * 2 + 1]);
      if (matchedx)
      {
        matchedxvec.push_back(i);
        hMMeff[0]->Fill(i);
      }
      if (matchedy)
      {
        matchedyvec.push_back(i);
        hMMeff[1]->Fill(i);
      }
      if (matchedy && matchedx)
      {
        matchedandvec.push_back(i);
        hMMeff[2]->Fill(i);
      }
    }
    hTrackereff[0]->Fill(matchedxvec.size());
    if (matchedxvec.size())
    {
      matchedxcounter++;
      matchedtotalcounter++;
    }

    hTrackereff[1]->Fill(matchedyvec.size());
    if (matchedyvec.size())
    {
      matchedycounter++;
      matchedtotalcounter++;
    }

    hTrackereff[2]->Fill(matchedandvec.size());
    if (matchedandvec.size())
    {
      matchedandcounter++;
    }
    hTrackereff[3]->Fill(matchedxvec.size() + matchedyvec.size());
  }

  int ValidTracker[4];
  ValidTracker[0] = hTrackereff[0]->Integral(2, 100);
  ValidTracker[1] = hTrackereff[1]->Integral(2, 100);
  ValidTracker[2] = hTrackereff[2]->Integral(2, 100);
  ValidTracker[3] = hTrackereff[3]->Integral(2, 100);
  cout << " --> Trigger Start = " << RecordStart << ", Trigger End =" << RecordEnd << endl
       << " --> Trigger coincidence: " << NumofTriggers << endl
       << " --> Valid tracker x counter: " << ValidTracker[0] << ", GE: " << (double)ValidTracker[0] / NumofTriggers << endl
       << " --> Valid tracker y counter: " << ValidTracker[1] << ", GE: " << (double)ValidTracker[1] / NumofTriggers << endl
       << " --> Valid tracker (x&y) counter: " << ValidTracker[2] << ", GE: " << (double)ValidTracker[2] / NumofTriggers << endl
       << " --> Valid tracker counter (or): " << ValidTracker[3] << ", GE: " << (double)ValidTracker[3] / NumofTriggers << endl;
  op << " --> Trigger Start = " << RecordStart << ", Trigger End =" << RecordEnd << endl
     << " --> Trigger coincidence: " << NumofTriggers << endl
     << " --> Valid tracker x counter: " << ValidTracker[0] << ", GE: " << (double)ValidTracker[0] / NumofTriggers << endl
     << " --> Valid tracker y counter: " << ValidTracker[1] << ", GE: " << (double)ValidTracker[1] / NumofTriggers << endl
     << " --> Valid tracker (x&y) counter: " << ValidTracker[2] << ", GE: " << (double)ValidTracker[2] / NumofTriggers << endl
     << " --> Valid tracker counter (or): " << ValidTracker[3] << ", GE: " << (double)ValidTracker[3] / NumofTriggers << endl;

  TCanvas *cHit = new TCanvas("Hit", "Hit");
  cHit->Divide(4, 2);
  for (int i = 0; i < 4; i++)
  {
    cHit->cd(i + 1);
    hHit[2 * i]->Draw();
    cHit->cd(i + 5);
    hHit[2 * i + 1]->Draw();
  }
  cHit->Write();

  TCanvas *cEff = new TCanvas("MMeff", "MMeff");
  cEff->Divide(4, 2);
  for (int s = 0; s < 3; s++)
  {
    cEff->cd(s + 1);
    hMMeff[s]->Draw();
    double MMeff[4];
    for (int i = 0; i < 4; i++)
    {
      MMeff[i] = hMMeff[s]->GetBinContent(i + 1) / NumofTriggers;
      TString latexbuff = Form("MMID=%d,Ratio=%.1f%%", i, MMeff[i] * 100);
      TLatex *la = CreatLatex(latexbuff, 0.2, 0.7 - 0.07 * i, 42);
      la->Draw("same");
    }
  }

  for (int s = 0; s < 4; s++)
  {
    cEff->cd(s + 5);
    hTrackereff[s]->Draw();
    int total = 0;
    if (s < 3)
      total = 5;
    else
      total = 9;
    double Trackereff[9];
    for (int i = 0; i < total; i++)
    {
      Trackereff[i] = hTrackereff[s]->GetBinContent(i + 1) / NumofTriggers;
      TString latexbuff;
      if (s == 3)
        latexbuff = Form("Nchips=%d,Ratio=%.1f%%", i, Trackereff[i] * 100);
      else
        latexbuff = Form("NMM=%d,Ratio=%.1f%%", i, Trackereff[i] * 100);
      TLatex *la = CreatLatex(latexbuff, 0.2, 0.7 - 0.05 * i, 42);
      la->Draw("same");
    }
  }
  cEff->Write();

  fanaFile->Close();
  frawFile->Close();
  cout << " --> Analyzed results has been stored to " << anaName << endl;
  op << " --> Analyzed results has been stored to " << anaName << endl;
  op.close();
}

int AnalysisAPV::IndexXY(int board, int chip)
{
  // int index = -1;
  // if (chip == 10 || chip == 11) index = 1; //y
  // if (chip == 12 || chip == 13) index = 0; //x
  return chip / 2;
}

int AnalysisAPV::IndexDet(int board, int chip)
{
  // int index = -1;
  // if (board == 1 && (chip == 12 || chip == 11)) index = 0;
  // if (board == 1 && (chip == 10 || chip == 13)) index = 1;
  // if (board == 2 && (chip == 12 || chip == 11)) index = 2;
  // if (board == 2 && (chip == 10 || chip == 13)) index = 3;
  return board;
}

int AnalysisAPV::IndexStrip(int board, int chip, int channel)
{
  return APVmap.at((board * 4 + chip) * 128 + channel);
}

int AnalysisAPV::CalculateLostID(vector<int> eventvec)
{
  int start = 1;
  int lostcounter = 0;
  while (start < eventvec.size())
  {
    int l = abs(eventvec.at(start) - eventvec.at(start - 1));
    if (l != 1 && l < 50) // if l>50, may be stop record
    {
      for (int i = 1; i < l; i++)
      {
        lostcounter++;
        cout << " --> lost event " << eventvec.at(start - 1) + i << " between (" << eventvec.at(start - 1) << "," << eventvec.at(start) << ") !" << endl;
      }
    }
    start++;
  }
  return lostcounter;
}

void AnalysisAPV::CalWaveInfo()
{
  int peakstart = 0;
  int peakend = (int)wave->size();
  int threshold = -384;
  maxA = -65536;
  for (int i = peakstart; i < peakend; i++)
  {
    if (maxA < (double)wave->at(i))
    {
      maxA = (double)wave->at(i);
      maxT = i;
    }
  }

  for (int i = maxT; i > peakstart; i--)
  {
    if ((double)wave->at(i) < threshold)
    {
      peakstart = max(i - 4, 0);
      break;
    }
  }
  for (int i = maxT; i < peakend; i++)
  {
    if ((double)wave->at(i) < threshold)
    {
      peakend = min(i + 4, peakend);
      break;
    }
  }
  for (int i = 0; i < peakstart; i++)
  {
    ped += (double)wave->at(i);
    pedrms += pow((double)wave->at(i), 2);
  }
  for (int i = peakend; i < wave->size(); i++)
  {
    ped += (double)wave->at(i);
    pedrms += pow((double)wave->at(i), 2);
  }
  int n_wave = (int)wave->size() - peakend + peakstart;
  ped /= n_wave;
  pedrms = TMath::Sqrt(pedrms / n_wave - ped * ped);
  // for (int kk = peakstart; kk < peakend; kk++)
  // {
  //   cout << "wave " << (double)wave->at(kk) << endl;
  // }
  maxA -= ped;
  // cout << maxA << " " << maxT << " ped " << ped << endl;
  // cout << "waveend" << endl;
}

void AnalysisAPV::InitData()
{
  maxA = 0;
  maxT = 0;
  ped = 0;
  pedrms = 0;
  riseT = 0;
  TOT = 0;
}

void AnalysisAPV::riseTandTOT(vector<double> waveform)
{
  if (maxA < pedrms * rmscut)
  {
    InitData();
    return;
  }

  double cfamp[4] = {0.2 * maxA, 0.8 * maxA, 0.5 * maxA, 0.5 * maxA};
  double stamp[4] = {double(maxT) - 1, double(maxT), double(maxT), double(maxT) + 1};

  // find start 0.2*maxA
  for (int i = maxT; i > 0; i--)
    if (waveform[i] - cfamp[0] > 0 && waveform[i - 1] - cfamp[0] < 0)
    {
      stamp[0] = i;
      break;
    }
  // find start 0.8*maxA
  for (int i = maxT; i > 0; i--)
    if (waveform[i] - cfamp[1] > 0 && waveform[i - 1] - cfamp[1] < 0)
    {
      stamp[1] = i;
      break;
    }
  // find start 0.5*maxA
  for (int i = maxT; i > 0; i--)
    if (waveform[i] - cfamp[2] > 0 && waveform[i - 1] - cfamp[2] < 0)
    {
      stamp[2] = i;
      break;
    }
  // find end 0.5*maxA
  for (int i = maxT; i < waveform.size() - 1; i++)
    if (waveform[i] - cfamp[3] > 0 && waveform[i + 1] - cfamp[3] < 0)
    {
      stamp[3] = i + 1;
      break;
    }
  double time[4];
  for (int i = 0; i < 4; i++)
  {
    double y1 = stamp[i] - 1;
    double y2 = stamp[i];
    double x1 = waveform[stamp[i] - 1];
    double x2 = waveform[stamp[i]];
    time[i] = (y2 - y1) * (cfamp[i] - x1) / (x2 - x1) + y1;
  }
  riseT = time[1] - time[0];
  TOT = time[3] - time[2];
}
