#include "DecodeAPV.h"

DecodeAPV::DecodeAPV(TString fanaName, TString fdecName, int f_force)
{
  anaName = fanaName;
  decName = fdecName;
  force = f_force;
}

DecodeAPV::~DecodeAPV()
{
}

bool DecodeAPV::Cut()
{
  return maxA > pedrms * rmscut && maxT > 0 && maxT < 30 && riseT > 0 && riseT < 10 && TOT > 0 && TOT < 15;
}

bool DecodeAPV::Init()
{
  fanaFile = new TFile(anaName, "read");
  if (!fanaFile->IsOpen())
  {
    cout << " ERROR! " << anaName << " cant open!" << endl;
    return false;
  }
  cout << " --> Decoding strips from " << anaName << endl;
  for (int i = 0; i < 8; i++)
  {
    fanaTree[i] = (TTree *)fanaFile->Get(Form("Track%s%d", XYname(i % 2).Data(), i / 2));
    if (fanaTree[i] == 0)
    {
      cout << " ERROR! Track" << XYname(i % 2) << i / 2 << " Tree not exist in " << anaName << endl;
      return false;
    }
    fanaTree[i]->SetBranchAddress("event", &event, &b_event);
    fanaTree[i]->SetBranchAddress("maxA", &maxA, &b_maxA);
    fanaTree[i]->SetBranchAddress("maxT", &maxT, &b_maxT);
    fanaTree[i]->SetBranchAddress("channel", &channel, &b_channel);
    fanaTree[i]->SetBranchAddress("ped", &ped, &b_ped);
    fanaTree[i]->SetBranchAddress("pedrms", &pedrms, &b_pedrms);
    fanaTree[i]->SetBranchAddress("riseT", &riseT, &b_riseT);
    fanaTree[i]->SetBranchAddress("TOT", &TOT, &b_TOT);
  }

  if (force != 1)
  {
    FileStat_t fStat;
    gSystem->GetPathInfo(decName, fStat);
    if (fStat.fSize != 0)
    {
      cout << " ERROR! " << decName << " is exist!" << endl;
      return false;
    }
  }
  fdecFile = new TFile(decName, "recreate");
  if (!fdecFile->IsOpen())
  {
    cout << "ERROR! " << decName << " cant open!" << endl;
    return false;
  }
  for (int i = 0; i < 4; i++)
  {
    fdecTree[i] = new TTree(Form("Track%d", i), Form("Track%d", i));
    fdecTree[i]->Branch("event", &event);
    fdecTree[i]->Branch("sig", &sig);
    fdecTree[i]->Branch("sig_x", &sig_x);
    fdecTree[i]->Branch("sig_y", &sig_y);
    fdecTree[i]->Branch("x", &x);
    fdecTree[i]->Branch("y", &y);
    fdecTree[i]->Branch("z", &z);
    fdecTree[i]->Branch("hit_amp_x", &hit_amp_x);
    fdecTree[i]->Branch("hit_amp_y", &hit_amp_y);
    fdecTree[i]->Branch("hit_strip_num_x", &hit_strip_num_x);
    fdecTree[i]->Branch("hit_strip_num_y", &hit_strip_num_y);
    fdecTree[i]->Branch("x_nhits", &x_nhits);
    fdecTree[i]->Branch("y_nhits", &y_nhits);
    fdecTree[i]->Branch("x_other", &x_other);
    fdecTree[i]->Branch("y_other", &y_other);
    fdecTree[i]->Branch("vec_Cluster_x", &vec_Cluster_x);
    fdecTree[i]->Branch("vec_Cluster_y", &vec_Cluster_y);
  }

  // dechit = new APV2DetStrip(asic2detName);

  return true;
}

void DecodeAPV::Loop()
{
  ofstream op = LogFile(decName);
  TCanvas *cTmp = new TCanvas("tmpraw", "tmpraw");
  TH1F *hx[4];
  TH1F *hy[4];
  TH2F *hxy[4];
  TH1F *hampx[4];
  TH1F *hampy[4];
  TH1F *hampxy[4];
  TH1I *hstripx[4];
  TH1I *hstripy[4];
  for (int i = 0; i < 4; i++)
  {
    hx[i] = new TH1F(Form("hitx%d", i), Form("hitx%d", i), 100, -100, 100);
    hx[i]->GetXaxis()->SetTitle("X [mm]");
    hx[i]->GetYaxis()->SetTitle("Counts");
    hy[i] = new TH1F(Form("hity%d", i), Form("hity%d", i), 100, -100, 100);
    hy[i]->GetXaxis()->SetTitle("Y [mm]");
    hy[i]->GetYaxis()->SetTitle("Counts");
    hxy[i] = new TH2F(Form("hitxy%d", i), Form("hitxy%d", i), 100, -100, 100, 100, -100, 100);
    hxy[i]->GetXaxis()->SetTitle("X [mm]");
    hxy[i]->GetYaxis()->SetTitle("Y [mm]");
    hampx[i] = new TH1F(Form("ampx%d", i), Form("ampx%d", i), 100, 0, 10000);
    hampx[i]->GetXaxis()->SetTitle("amplitude X [ADC]");
    hampx[i]->GetYaxis()->SetTitle("Counts");
    hampy[i] = new TH1F(Form("ampy%d", i), Form("ampy%d", i), 100, 0, 10000);
    hampy[i]->GetXaxis()->SetTitle("amplitude Y [ADC]");
    hampy[i]->GetYaxis()->SetTitle("Counts");
    hampxy[i] = new TH1F(Form("ampxy%d", i), Form("ampxy%d", i), 100, 0, 20000);
    hampxy[i]->GetXaxis()->SetTitle("amplitude X+Y [ADC]");
    hampxy[i]->GetYaxis()->SetTitle("Counts");
    hstripx[i] = new TH1I(Form("Nstripx%d", i), Form("Nstripx%d", i), 20, 0, 20);
    hstripx[i]->GetXaxis()->SetTitle("Num of fired X-strips");
    hstripx[i]->GetYaxis()->SetTitle("Counts");
    hstripy[i] = new TH1I(Form("Nstripy%d", i), Form("Nstripy%d", i), 20, 0, 20);
    hstripy[i]->GetXaxis()->SetTitle("Num of fired Y-strips");
    hstripy[i]->GetYaxis()->SetTitle("Counts");
  }

  /////////// begin to loop
  vector<int> eventvec[8];
  int min[8], max[8];
  for (int i = 0; i < 8; i++)
  {
    int nentries = fanaTree[i]->GetEntriesFast();
    for (int j = 0; j < nentries; j++)
    {
      fanaTree[i]->GetEntry(j);
      eventvec[i].push_back(event);
    }
    min[i] = eventvec[i][0];
    max[i] = event;
    cout << " --> Tracker" << XYname(i % 2) << i / 2 << ", Events: " << nentries
         << ", trigger range: " << min[i] << " to " << max[i] << endl;
    op << " --> Tracker" << XYname(i % 2) << i / 2 << ", Events: " << nentries
       << ", trigger range: " << min[i] << " to " << max[i] << endl;
  }
  int start = TMath::MinElement(8, min);
  int end = TMath::MaxElement(8, max);
  cout << " --> Searching trigger from " << start << " to " << end << endl;
  op << " --> Searching trigger from " << start << " to " << end << endl;

  /// event loop
  for (int i = start; i <= end; i++)
  {
    DrawProcessbar(i - start, end - start + 1);

    int pos[8];
    int goodNum = 0;
    for (int j = 0; j < 8; j++)
    {
      pos[j] = IsMatched(i, eventvec[j]); /// eventvec[pos-1] == i
      if (pos[j])
        goodNum++;
    }
    if (!goodNum)
    {
      InitData();
      for (int j = 0; j < 8; j++)
      {
        fdecTree[j / 2]->Fill();
      }
      continue;
    }

    for (int j = 0; j < 8; j++)
    {
      if (j % 2 == 0)
        InitData();
      if (pos[j] != 0)
      {
        vector<int> aget_ch;
        vector<double> aget_amp;
        std::vector<std::pair<int, double>> paired_vector;
        while (i == eventvec[j][pos[j] - 1])
          pos[j]--; // find first pos, eventvec[pos] == i

        int nentries = fanaTree[j]->GetEntriesFast();
        for (int k = pos[j]; k < nentries; k++)
        {
          fanaTree[j]->GetEntry(k);
          if (event != i)
          {
            event = i;
            break;
          }
          if (Cut()) // Cut()
          {
            // aget_ch.push_back(channel);
            // aget_amp.push_back(maxA);
            paired_vector.push_back(std::make_pair(channel, maxA));
          }
        }

        if (paired_vector.size() != 0)
        { ///////////////recode

          //************************************************************************ */
          // 根据第一个向量的值进行排序
          std::sort(paired_vector.begin(), paired_vector.end());

          for (auto &&ii : paired_vector)
          {
            aget_ch.push_back(ii.first);
            aget_amp.push_back(ii.second);
          }

          // cout << endl
          //      << "event: " << i << endl;
          // for (size_t ii = 0; ii < aget_ch.size(); ii++)
          // {
          //   cout << "channel: " << aget_ch[ii] << " amp: " << aget_amp[ii] << endl;
          // }

          if (CalHit(aget_ch, aget_amp, j))
          {
            if (j % 2 == 0)
            {
              if (x > 0)
              {
                hx[j / 2]->Fill((x - 256 / 2) * 0.4); // total strips 384
                hampx[j / 2]->Fill(hit_amp_x);
              }
              hstripx[j / 2]->Fill(hit_strip_num_x);
            }

            if (j % 2 == 1)
            {
              if (y > 0)
              {
                hy[j / 2]->Fill((y - 256 / 2) * 0.4); // total strips 384
                hampy[j / 2]->Fill(hit_amp_y);
                hampxy[j / 2]->Fill(hit_amp_x + hit_amp_y);
              }
              hstripy[j / 2]->Fill(hit_strip_num_y);
            }

            sig = sig_x && sig_y;
            if (sig && x > 0 && y > 0)
              hxy[j / 2]->Fill((x - 256 / 2) * 0.4, (y - 256 / 2) * 0.4);
            z = Zpos[j / 2];
          }
          else
          {
            // cout<<" --> Decode failed Tracker"<<XYname(j%2)<<j/2<<", event ID: "<<event<< endl;
            // op<<" --> Decode failed Tracker"<<XYname(j%2)<<j/2<<", event ID: "<<event<< endl;
          }
          //**************************************************************************************** */
        }
        else
        {
          event = i;
          // cout<<" --> No record hit at Tracker"<<XYname(j%2)<<j/2<<", event ID: "<<event<< endl;
          // op<<" --> No record hit at Tracker"<<XYname(j%2)<<j/2<<", event ID: "<<event<< endl;
        }
      }
      else
      {
        event = i;
        // cout<<" --> Lost record hit at Tracker"<<XYname(j%2)<<j/2<<", event ID: "<<event<< endl;
        op << " --> Lost record hit at Tracker" << XYname(j % 2) << j / 2 << ", event ID: " << event << endl;
      }
      if (j % 2 == 1)
      {
        fdecTree[j / 2]->Fill();
      }
    }
  } // event loop
  for (int i = 0; i < 4; i++)
    fdecFile->WriteTObject(fdecTree[i]);

  TCanvas *cMMhit = new TCanvas("MMhit", "MMhit");
  cMMhit->Divide(4, 3);
  for (int i = 0; i < 4; i++)
  {
    cMMhit->cd(i + 1);
    hx[i]->Draw();
    cMMhit->cd(i + 5);
    hy[i]->Draw();
    cMMhit->cd(i + 9);
    hxy[i]->Draw("COLZ");
  }
  cMMhit->Write();

  TCanvas *cMMamp = new TCanvas("MMamp", "MMamp");
  cMMamp->Divide(4, 5);
  TLatex *la;
  for (int i = 0; i < 4; i++)
  {
    cMMamp->cd(i + 1);
    hampx[i]->Fit("landau", "Q");
    hampx[i]->Draw();
    double peakx = hampx[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peakx), 0.65, 0.5);
    la->Draw("same");
    cMMamp->cd(i + 5);
    hampy[i]->Fit("landau", "Q");
    hampy[i]->Draw();
    double peaky = hampy[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peaky), 0.65, 0.5);
    la->Draw("same");
    cMMamp->cd(i + 9);
    hampxy[i]->Fit("landau", "Q");
    hampxy[i]->Draw();
    double peakxy = hampxy[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peakxy), 0.65, 0.5);
    la->Draw("same");
    cMMamp->cd(i + 13);
    hstripx[i]->Draw();
    cMMamp->cd(i + 17);
    hstripy[i]->Draw();
  }
  cMMamp->Write();

  TCanvas *cMMamp2 = new TCanvas("MMamp2", "MMamp2");
  cMMamp2->Divide(4, 3);
  for (int i = 0; i < 4; i++)
  {
    cMMamp2->cd(i + 1);
    hampx[i]->Fit("landau", "Q");
    hampx[i]->Draw();
    double peakx = hampx[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peakx), 0.65, 0.5);
    la->Draw("same");
    cMMamp2->cd(i + 5);
    hampy[i]->Fit("landau", "Q");
    hampy[i]->Draw();
    double peaky = hampy[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peaky), 0.65, 0.5);
    la->Draw("same");
    cMMamp2->cd(i + 9);
    hampxy[i]->Fit("landau", "Q");
    hampxy[i]->Draw();
    double peakxy = hampxy[i]->GetFunction("landau")->GetParameter(1);
    la = CreatLatex(Form("MPV=%.f", peakxy), 0.65, 0.5);
    la->Draw("same");
  }
  cMMamp2->Write();

  fanaFile->Close();
  fdecFile->Close();
  cout << " --> Decoding results has been stored to " << decName << endl;
  op << " --> Decoding results has been stored to " << decName << endl;
}

void DecodeAPV::InitData()
{
  sig = false;
  sig_x = false;
  sig_y = false;
  x = 0;
  y = 0;
  z = 0;
  hit_amp_x = 0;
  hit_amp_y = 0;
  hit_strip_num_x = 0;
  hit_strip_num_y = 0;
  x_nhits = 0;
  y_nhits = 0;
  x_other.clear();
  y_other.clear();
  vec_Cluster_x.clear();
  vec_Cluster_y.clear();
}

bool DecodeAPV::CalHit(vector<int> ch, vector<double> amp, int XY)
{                             /// waves are easy to maximum
  double overthreshold = 100; // maxA should be lower than that
  if ((ch.size() == 0) || (ch.size() > 100))
    return false;
  double temp = 0;
  double ttemp = 0;
  vector<double> vec_temp;
  int locmax = 0;
  int channelloc = 0;
  double Q1, Q2;
  double m, n;
  double Qall = 0;
  double tQall = 0;
  vector<double> vec_Qall;
  int pre_channel;
  double Q0 = overthreshold;
  double A, B, C, D, E, F, G, H;
  if (0) // TMath::MaxElement(amp.size(), &amp.at(0)) > 1200
  {
    if (ch.size() == 1)
    {
      temp = ch.at(0);
    }
    else if (ch.size() == 2)
    {
      temp = (ch.at(0) + ch.at(1)) / 2;
    }
    else
    {
      int channelbegin = ch.at(0);
      int channelend = ch.at(ch.size() - 1);
      // locmax = TMath::LocMax(amp.size(), &amp.at(0));
      // for (int i = locmax; i < 256; i++)
      // {
      //   if (amp.at(i) < overthreshold)
      //   {
      //     channelend = i;
      //     break;
      //   }
      // }
      // for (int i = locmax; i > 0; i--)
      // {
      //   if (amp.at(i) < overthreshold)
      //   {
      //     channelbegin = i;
      //     break;
      //   }
      // }
      if (ch.size() > 1)
      {
        temp = abs(channelend - channelbegin) / 2.0;
        // A = (overthreshold - amp.at(max(0, channelbegin - 2))) / 2.0;
        // B = (overthreshold - amp.at(min(channelend + 2, 30))) / 2.0;
        // temp = (channelbegin * A - channelend * B) / (A - B);
      }
    }
  }
  else
  {
    pre_channel = ch[0];
    for (int i = 0; i < ch.size(); i++)
    {

      temp += amp.at(i) * ch.at(i);
      Qall += amp.at(i);
      ttemp += amp.at(i) * ch.at(i);
      tQall += amp.at(i);
      if ((ch[i] - pre_channel > 3) || (i == (ch.size() - 1)))
      {
        vec_Qall.push_back(tQall);
        vec_temp.push_back(ttemp /= tQall);
        tQall = 0;
        ttemp = 0;
      }
      pre_channel = ch.at(i);
    }
    // for (size_t i = 0; i < vec_Qall.size(); i++)
    // {
    //   cout << " cluster: " << i << " position: " << vec_temp[i] << " Q: " << vec_Qall[i] << endl;
    // }

    temp /= Qall; // average
  }
  // cout<<"data : "<<endl;
  // for(int i=0;i<ch.size();i++){
  //   cout<<"channel "<<ch.at(i)<<" and amp "<<amp.at(i)<<endl;
  // }
  // cout<<"event end"<<endl;

  if (XY % 2 == 0)
  {
    sig_x = true;
    x = temp;
    temp = 0;
    for (auto &&ii : vec_temp)
    {
      vec_Cluster_x.push_back(ii);
    }

    vec_temp.clear();
    // x-=128/2;
    // hit_amp_x = TMath::MaxElement(amp.size(), &amp.at(0));
    hit_amp_x = Qall;
    hit_strip_num_x = amp.size();
    for (int s = 1; s < ch.size() && x_nhits < 5; s++, x_nhits++)
      x_other.push_back(ch.at(s));
    Qall = 0;
    vec_Qall.clear();
  }
  if (XY % 2 == 1)
  {
    sig_y = true;
    y = temp;
    temp = 0;
    for (auto &&ii : vec_temp)
    {
      vec_Cluster_y.push_back(ii);
    }
    vec_temp.clear();
    // hit_amp_y = TMath::MaxElement(amp.size(), &amp.at(0));
    hit_amp_y = Qall;
    hit_strip_num_y = amp.size();
    for (int s = 1; s < ch.size() && y_nhits < 5; s++, y_nhits++)
      y_other.push_back(ch.at(s));
    Qall = 0;
    vec_Qall.clear();
  }

  return true;
}