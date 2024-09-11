#include "ReconstructTrack.h"

using namespace TMath;

ReconstructTrack::ReconstructTrack(TString fdecName, TString frecName, int f_force)
{
  decName = fdecName;
  recName = frecName;
  force = f_force;
  alignmentPar3Name = MMalignmentPar3Name;
  alignmentPar6Name = MMalignmentPar6Name;
}

ReconstructTrack::~ReconstructTrack()
{
}

bool ReconstructTrack::Init()
{
  fdecFile = new TFile(decName, "read");
  if (!fdecFile->IsOpen())
  {
    cout << " ERROR! " << decName << " cant open!" << endl;
    return false;
  }
  cout << " --> Reconstruct track from " << decName << endl;
  for (int i = 0; i < MMdetN; i++)
  {
    fdecTree[i] = (TTree *)fdecFile->Get(Form("Track%d", i));
    if (fdecTree[i] == 0)
    {
      cout << " ERROR! Track" << i << " Tree not exist in " << decName << endl;
      return false;
    }

    fdecTree[i]->SetBranchAddress("event", &event, &b_event);
    fdecTree[i]->SetBranchAddress("sig", &sig, &b_sig);
    fdecTree[i]->SetBranchAddress("sig_x", &sig_x, &b_sig_x);
    fdecTree[i]->SetBranchAddress("sig_y", &sig_y, &b_sig_y);
    fdecTree[i]->SetBranchAddress("x", &x, &b_x);
    fdecTree[i]->SetBranchAddress("y", &y, &b_y);
    fdecTree[i]->SetBranchAddress("z", &z, &b_z);
    // fdecTree[i]->SetBranchAddress("hit_amp_x", &hit_amp_x, &b_hit_amp_x);
    // fdecTree[i]->SetBranchAddress("hit_amp_y", &hit_amp_y, &b_hit_amp_y);
    // fdecTree[i]->SetBranchAddress("hit_strip_num_x", &hit_strip_num_x, &b_hit_strip_num_x);
    // fdecTree[i]->SetBranchAddress("hit_strip_num_y", &hit_strip_num_y, &b_hit_strip_num_y);
    // fdecTree[i]->SetBranchAddress("x_nhits", &x_nhits, &b_x_nhits);
    // fdecTree[i]->SetBranchAddress("y_nhits", &y_nhits, &b_y_nhits);
    // fdecTree[i]->SetBranchAddress("x_other", &x_other, &b_x_other);
    // fdecTree[i]->SetBranchAddress("y_other", &y_other, &b_y_other);
  }

  if (force != 1)
  {
    FileStat_t fStat;
    gSystem->GetPathInfo(recName, fStat);
    if (fStat.fSize != 0)
    {
      cout << " ERROR! " << recName << " is exist!" << endl;
      return false;
    }
  }
  frecFile = new TFile(recName, "recreate");
  if (!frecFile->IsOpen())
  {
    cout << "ERROR! " << recName << " cant open!" << endl;
    return false;
  }
  frecTree = new TTree("Track", "Track");
  frecTree->Branch("event", &event, "event/I");
  frecTree->Branch("MMx", MMx, Form("MMx[%d]/D", MMdetN));
  frecTree->Branch("MMy", MMy, Form("MMy[%d]/D", MMdetN));
  frecTree->Branch("MMz", MMz, Form("MMz[%d]/D", MMdetN));
  frecTree->Branch("MMsigx", MMsigx, Form("MMsigx[%d]/O", MMdetN));
  frecTree->Branch("MMsigy", MMsigy, Form("MMsigy[%d]/O", MMdetN));
  frecTree->Branch("XfitPars", XfitPars, "XfitPars[3]/D");
  frecTree->Branch("YfitPars", YfitPars, "YfitPars[3]/D");
  frecTree->Branch("useflag", &useflag, "useflag/O");

  // get_APV2det_lists(asic2detName.Data());

  cal_APV2det_lists();
  alignmentPar3Name = GetPath(recName) + "/MMalignmentPar3.dat";
  alignmentPar6Name = GetPath(recName) + "/MMalignmentPar6.dat";
  ReadAlignmentPars(alignmentPar3Name.Data(), 3);
  ReadAlignmentPars(alignmentPar6Name.Data(), 6);
  return true;
}

void ReconstructTrack::Loop()
{
  ofstream op = LogFile(recName);
  TCanvas *cTmp = new TCanvas("rectmp", "rectmp");
  TH1F *hpos[MMdetN][2];
  TH1F *htheta[MMdetN][3];
  for (int i = 0; i < MMdetN; i++)
  {
    hpos[i][0] = new TH1F(Form("resX%d", i), Form("resX%d", i), 500, -4000, 4000);
    hpos[i][0]->GetXaxis()->SetTitle("x [#mum]");
    hpos[i][0]->GetYaxis()->SetTitle("counts");
    hpos[i][1] = new TH1F(Form("resY%d", i), Form("resY%d", i), 500, -4000, 4000);
    hpos[i][1]->GetXaxis()->SetTitle("y [#mum]");
    hpos[i][1]->GetYaxis()->SetTitle("counts");
    htheta[i][0] = new TH1F(Form("thetaX%d", i), Form("thetaX%d", i), 100, 0, 5);
    htheta[i][0]->GetXaxis()->SetTitle("#theta_{x} [#circ]");
    htheta[i][0]->GetYaxis()->SetTitle("counts");
    htheta[i][1] = new TH1F(Form("thetaY%d", i), Form("thetaY%d", i), 100, 0, 5);
    htheta[i][1]->GetXaxis()->SetTitle("#theta_{y} [#circ]");
    htheta[i][1]->GetYaxis()->SetTitle("counts");
    htheta[i][2] = new TH1F(Form("thetaMM%d", i), Form("thetaMM%d", i), 100, 0, 5);
    htheta[i][2]->GetXaxis()->SetTitle("#theta [#circ]");
    htheta[i][2]->GetYaxis()->SetTitle("counts");
  }

  double res[MMdetN][2] = {0};
  double efficiencyX[MMdetN][3] = {0};
  double efficiencyY[MMdetN][3] = {0};
  int count[MMdetN][2] = {0};
  int nentries = fdecTree[0]->GetEntriesFast();
  for (int i = 0; i < nentries; i++)
  {
    DrawProcessbar(i, nentries);
    InitData();
    vector<double> px, py, pz;
    for (int j = 0; j < MMdetN; j++)
    {
      fdecTree[j]->GetEntry(i);
      px.push_back(x);
      py.push_back(y);
      pz.push_back(z);
    }
    if (track_reconstruction(px, py, pz))
      frecTree->Fill();
    else
    {
      InitData();
      useflag = 0;
      frecTree->Fill();
    }

    for (int j = 0; j < MMdetN; j++)
    {
      vector<double> xx, yy, zx, zy;
      double parx[3], pary[3];
      for (int k = 0; k < MMdetN; k++)
      {
        if (k != j)
        {
          if (MMsigx[k])
          {
            xx.push_back(MMx[k]);
            zx.push_back(MMz[k]);
          }
          if (MMsigy[k])
          {
            yy.push_back(MMy[k]);
            zy.push_back(MMz[k]);
          }
        }
      }
      bool good_fit_x = false;
      bool good_fit_y = false;
      if (fit_track(zx, xx, parx))
      {
        good_fit_x = true;
        count[j][0]++;
        double thetaX = Abs(ATan(parx[0]) / Pi() * 180);
        double fitX = parx[0] * MMz[j] + parx[1];
        htheta[j][0]->Fill(thetaX);
        if (thetaX < theta_cut && Abs(fitX) < pos_cut)
        {
          efficiencyX[j][0]++;
          if (MMsigx[j] && Abs(MMx[j] - fitX) < 100)
          {
            efficiencyX[j][1]++;
            hpos[j][0]->Fill((MMx[j] - fitX) * 1000);
          }
        }
      }
      if (fit_track(zy, yy, pary))
      {
        good_fit_y = true;
        count[j][1]++;
        double thetaY = Abs(ATan(pary[0]) / Pi() * 180);
        double fitY = pary[0] * MMz[j] + pary[1];
        htheta[j][1]->Fill(thetaY);
        if (thetaY < theta_cut && Abs(fitY) < pos_cut)
        {
          efficiencyY[j][0]++;
          if (MMsigy[j] && Abs(MMy[j] - fitY) < 100)
          {
            efficiencyY[j][1]++;
            hpos[j][1]->Fill((MMy[j] - fitY) * 1000);
          }
        }
      }
      if (good_fit_x && good_fit_y)
        htheta[j][2]->Fill(ATan(Sqrt(parx[0] * parx[0] + pary[0] * pary[0])) / Pi() * 180);
    }
  }
  frecTree->Write();

  for (int i = 0; i < MMdetN; i++)
  {
    hpos[i][0]->Fit("gaus", "QR", "", -1200, 1200);
    res[i][0] = hpos[i][0]->GetFunction("gaus")->GetParameter(2);
    hpos[i][1]->Fit("gaus", "QR", "", -1200, 1200);
    res[i][1] = hpos[i][1]->GetFunction("gaus")->GetParameter(2);
    efficiencyX[i][2] = efficiencyX[i][1] / efficiencyX[i][0];
    efficiencyY[i][2] = efficiencyY[i][1] / efficiencyY[i][0];
  }

  TCanvas *cMMeff = new TCanvas("Trackereff", "Trackereff");
  cMMeff->Divide(2, MMdetN);
  TLatex *la;
  for (int i = 0; i < MMdetN; i++)
  {
    cout << " --> Det = " << i << endl
         << " --> 3MM coincides, X: " << count[i][0] << ", Y:" << count[i][1] << endl
         << " --> fit good, X: " << efficiencyX[i][0] << ", Y:" << efficiencyY[i][0] << endl
         << " --> record, X: " << efficiencyX[i][1] << ", Y: " << efficiencyY[i][1] << endl
         << " --> efficiency, X: " << efficiencyX[i][2] << ", Y: " << efficiencyY[i][2] << endl
         << " --> resolution, X: " << res[i][0] << ", Y: " << res[i][1] << endl;
    op << " --> Det = " << i << endl
       << " --> 3MM coincides, X: " << count[i][0] << ", Y:" << count[i][1] << endl
       << " --> fit good, X: " << efficiencyX[i][0] << ", Y:" << efficiencyY[i][0] << endl
       << " --> record, X: " << efficiencyX[i][1] << ", Y: " << efficiencyY[i][1] << endl
       << " --> efficiency, X: " << efficiencyX[i][2] << ", Y: " << efficiencyY[i][2] << endl
       << " --> resolution, X: " << res[i][0] << ", Y: " << res[i][1] << endl;

    cMMeff->cd(2 * i + 1);
    hpos[i][0]->Draw();
    la = CreatLatex(Form("#color[2]{#sigma=%.0f#mum}", res[i][0]), 0.6, 0.35);
    la->Draw("same");
    la = CreatLatex(Form("Eff=%.1f%%", efficiencyX[i][2] * 100), 0.2, 0.75);
    la->Draw("same");

    cMMeff->cd(2 * i + 2);
    hpos[i][1]->Draw();
    la = CreatLatex(Form("#color[2]{#sigma=%.0f#mum}", res[i][1]), 0.6, 0.35);
    la->Draw("same");
    la = CreatLatex(Form("Eff=%.1f%%", efficiencyY[i][2] * 100), 0.2, 0.75);
    la->Draw("same");
  }
  cMMeff->Write();

  TCanvas *cTheta = new TCanvas("Mutheta", "Mutheta");
  cTheta->Divide(MMdetN, 3);
  for (int i = 0; i < MMdetN; i++)
  {
    cTheta->cd(i + 1);
    htheta[i][2]->Draw();
    cTheta->cd(i + MMdetN + 1);
    htheta[i][0]->Draw();
    cTheta->cd(i + 2 * MMdetN + 1);
    htheta[i][1]->Draw();
  }
  cTheta->Write();

  frecFile->Flush();
  frecFile->Close();
  op.close();
}

void ReconstructTrack::InitData()
{
  useflag = 1; // useful
  memset(MMx, 0, sizeof(MMx));
  memset(MMy, 0, sizeof(MMy));
  memset(MMz, 0, sizeof(MMz));
  memset(MMsigx, 0, sizeof(MMsigx));
  memset(MMsigy, 0, sizeof(MMsigy));
  memset(XfitPars, 0, sizeof(XfitPars));
  memset(YfitPars, 0, sizeof(YfitPars));
}

bool ReconstructTrack::track_reconstruction(vector<double> px, vector<double> py, vector<double> pz)
{
  if (pz.size() != MMdetN)
    return false;
  vector<int> idx, idy;
  vector<double> tmpzx, tmpx, tmpzy, tmpy;
  for (int i = 0; i < pz.size(); i++)
  {
    if (px[i] != 0)
    {
      idx.push_back(i);
      tmpx.push_back(px[i]);
      tmpzx.push_back(pz[i]);
    }
    if (py[i] != 0)
    {
      idy.push_back(i);
      tmpy.push_back(py[i]);
      tmpzy.push_back(pz[i]);
    }
    MMz[i] = Zpos[i];
  }

  double Rx = 0, Ry = 0;
  for (int i = 0; i <= int(tmpx.size()) - 3; i++)
  {
    vector<double> tmpxx = tmpx;
    Rx = hit_selection(i, idx, tmpzx, tmpxx, 0);
    if (Rx > fit_R2_cut)
    {
      tmpx = tmpxx;
      break;
    }
  }
  for (int i = 0; i <= int(tmpy.size()) - 3; i++)
  {
    vector<double> tmpyy = tmpy;
    Ry = hit_selection(i, idy, tmpzy, tmpyy, 1);
    if (Ry > fit_R2_cut)
    {
      tmpy = tmpyy;
      break;
    }
  }

  if (Rx <= fit_R2_cut || Ry <= fit_R2_cut)
    return false;

  for (int i = 0; i < tmpx.size(); i++)
  {
    if (tmpx[i] != 0)
    {
      MMx[idx[i]] = tmpx[i] * 0.4 - centre;
      MMsigx[idx[i]] = true;
      tmpx[i] = MMx[idx[i]] + algPar3[idx[i]][0];
      tmpzx[i] = tmpzx[i] + algPar3[idx[i]][2];
    }
    else
    {
      idx.erase(idx.begin() + i);
      tmpx.erase(tmpx.begin() + i);
      tmpzx.erase(tmpzx.begin() + i);
      i--;
    }
  }
  for (int i = 0; i < tmpy.size(); i++)
  {
    if (tmpy[i] != 0)
    {
      MMy[idy[i]] = tmpy[i] * 0.4 - centre;
      MMsigy[idy[i]] = true;
      tmpy[i] = MMy[idy[i]] + algPar3[idy[i]][1];
      tmpzy[i] = tmpzy[i] + algPar3[idy[i]][2];
    }
    else
    {
      idy.erase(idy.begin() + i);
      tmpy.erase(tmpy.begin() + i);
      tmpzy.erase(tmpzy.begin() + i);
      i--;
    }
  }
  fit_track(tmpzx, tmpx, XfitPars);
  fit_track(tmpzy, tmpy, YfitPars);
  for (int i = 0; i < MMdetN; i++)
  {
    if (MMx[i] == 0)
      MMx[i] = XfitPars[0] * MMz[i] + XfitPars[1] - algPar3[i][0];
    if (MMy[i] == 0)
      MMy[i] = YfitPars[0] * MMz[i] + YfitPars[1] - algPar3[i][1];
    px[i] = MMx[i] + algPar6[i][0] - MMy[i] * algPar6[i][5];
    py[i] = MMy[i] + algPar6[i][1] + MMx[i] * algPar6[i][5];
    pz[i] = MMz[i] + algPar6[i][2] - MMx[i] * algPar6[i][4] + MMy[i] * algPar6[i][3];
    MMx[i] = px[i];
    MMy[i] = py[i];
    MMz[i] = pz[i];
  }
  fit_track(pz, px, XfitPars);
  fit_track(pz, py, YfitPars);
  return true;
}

double ReconstructTrack::hit_selection(int eraseN, vector<int> id, vector<double> px, vector<double> &py, int xy)
{
  int n = id.size();
  if (eraseN == 0 || n < 4)
    return hit_reconstruction(id, px, py, xy);

  vector<int> tmpid;
  vector<double> tmpx, tmpy, besty;
  double R2 = 0, R2_tmp;
  int bestpos = -1;
  for (int i = 0; i < n; i++)
  {
    tmpid = id;
    tmpid.erase(tmpid.begin() + i);
    tmpx = px;
    tmpx.erase(tmpx.begin() + i);
    tmpy = py;
    tmpy.erase(tmpy.begin() + i);
    double R2_tmp = hit_selection(eraseN - 1, tmpid, tmpx, tmpy, xy);
    if (R2_tmp > R2)
    {
      bestpos = i;
      R2 = R2_tmp;
      besty = tmpy;
    }
  }
  if (bestpos != -1)
  {
    py = besty;
    py.insert(py.begin() + bestpos, 0);
  }
  return R2;
}

double ReconstructTrack::hit_reconstruction(vector<int> id, vector<double> px, vector<double> &py, int xy)
{
  int n = id.size();
  if (n < 3)
    return 0;
  int single_hit_id = -1;
  for (int i = 0; i < n; i++) // find first single hit
    if (py[i] < 0)
    {
      single_hit_id = i;
      break;
    }

  if (single_hit_id == -1)
  {
    vector<double> tmpx, tmpy;
    double par[3] = {0};
    for (int i = 0; i < n; i++)
    {
      if (py[i] != 0)
      {
        tmpx.push_back(px[i] + algPar3[id[i]][2]);                 // 2 for z
        tmpy.push_back(py[i] * 0.4 - centre + algPar3[id[i]][xy]); // xy=0 for x, xy=1 for y
      }
    }
    fit_track(tmpx, tmpy, par);
    return par[2];
  }
  else if (is_single_correct)
  {
    vector<double> tmpy, besty;
    double R2 = 0, R2_tmp;
    int ch = -py[single_hit_id] - 1;
    int N = APV2det[ch].size();
    for (int i = 0; i < N; i++)
    {
      tmpy = py;
      tmpy[single_hit_id] = APV2det[ch][i] + 0.5;
      R2_tmp = hit_reconstruction(id, px, tmpy, xy);
      if (R2_tmp > R2)
      {
        R2 = R2_tmp;
        besty = tmpy;
      }
    }
    py = besty;
    return R2;
  }
  else
  {
    vector<int> tmpid;
    vector<double> tmpx, tmpy;
    tmpid = id;
    tmpid.erase(tmpid.begin() + single_hit_id);
    tmpx = px;
    tmpx.erase(tmpx.begin() + single_hit_id);
    tmpy = py;
    tmpy.erase(tmpy.begin() + single_hit_id);
    double R2 = hit_reconstruction(tmpid, tmpx, tmpy, xy);
    py = tmpy;
    py.insert(py.begin() + single_hit_id, 0);
    return R2;
  }
}

bool ReconstructTrack::fit_track(vector<double> px, vector<double> py, double par[3])
{
  int n = px.size() < py.size() ? px.size() : py.size();
  if (n < 3)
    return false;
  double Mx = 0, My = 0, Mxx = 0, Myy = 0, Mxy = 0; // mean
  for (int i = 0; i < n; i++)
  {
    Mx += px[i] / n;
    My += py[i] / n;
    Mxx += px[i] * px[i] / n;
    Myy += py[i] * py[i] / n;
    Mxy += px[i] * py[i] / n;
  }
  /// y = K*x+B, R2 = correlation index squre
  par[0] = (Mxy - Mx * My) / (Mxx - Mx * Mx);                                     // K
  par[1] = My - par[0] * Mx;                                                      // B
  par[2] = (Mxy - Mx * My) * (Mxy - Mx * My) / (Mxx - Mx * Mx) / (Myy - My * My); // R2

  return par[2] > fit_R2_cut;
}

bool ReconstructTrack::get_APV2det_lists(string lists_name)
{
  ifstream lists(lists_name);
  string line;
  if (lists.is_open())
  {
    // Read the title line
    getline(lists, line);
    int asicCH = 0, detCH = 0;
    while (getline(lists, line))
    {
      istringstream line_stream(line);
      getline(line_stream, line, ',');
      if (asicCH != atoi(line.c_str()))
      {
        cout << " --> File " << lists_name << " content error!!!" << endl;
        break;
      }
      while (getline(line_stream, line, ','))
      {
        if (detCH = atoi(line.c_str()))
          APV2det[asicCH].push_back(detCH);
      }
      asicCH++;
    }
    if (asicCH != 64)
    {
      cout << lists_name << " --> content error!!! total lines: " << asicCH << endl;
      lists.close();
      return false;
    }
  }
  else
  {
    cout << " --> File " << lists_name << " open error!!!" << endl;
    lists.close();
    return false;
  }
  lists.close();
  /*for(int i=0; i<64; i++)
    {
    cout<<" --> AsicCH = "<<i<<", DetCH = ";
    for(int j=0; j<(int)APV2det[i].size(); j++)
    cout<<APV2det[i][j]<<" ";
    cout<<endl;
    }*/
  return true;
}

bool ReconstructTrack::cal_APV2det_lists()
{
  for (int i = 0; i < 256; i++)
    APV2det[i].push_back(i);
  return true;
}
void ReconstructTrack::ReadAlignmentPars(string filename, int Npars)
{
  for (int i = 0; i < MMdetN; i++)
    for (int j = 0; j < 6; j++)
    {
      if (Npars == 3)
        algPar3[i][j] = 0;
      if (Npars == 6)
        algPar6[i][j] = 0;
    }

  ifstream in_file(filename.c_str());
  // if(0)
  if (in_file)
  {
    cout << " --> Read alignment data from " << filename << endl;
    for (int i = 0; i < MMdetN; i++)
    {
      if (Npars == 3)
      {
        cout << " --> det =" << i;
        for (int j = 0; j < 6; j++)
        {
          in_file >> algPar3[i][j];
          cout << " " << ParName[j] << " " << algPar3[i][j];
        }
        cout << endl;
      }
      if (Npars == 6)
      {
        cout << " --> det=" << i;
        for (int j = 0; j < 6; j++)
        {
          in_file >> algPar6[i][j];
          cout << ", " << ParName[j] << "=" << algPar6[i][j];
        }
        cout << endl;
      }
    }
    cout << " --> Read Tracker alignment pars succesfully !" << endl
         << endl;
    in_file.close();
  }
  else
    Alignment(filename, Npars);
}

void ReconstructTrack::Alignment(string filename, int Npars)
{
  cout << " --> Begin to do tracker alignment !" << endl;

  double res = alg_res;                        // delta-residual resolution
  double rho = alg_rho, epsilon = alg_epsilon; // for alginment loop
  vector<double> xvec[MMdetN], yvec[MMdetN], zvec[MMdetN];
  int nentries = fdecTree[0]->GetEntriesFast();
  for (int i = 0; i < nentries; i++)
  {
    bool is_good = true;
    double posX[MMdetN], posY[MMdetN], posZ[MMdetN];
    for (int j = 0; j < MMdetN; j++)
    {
      fdecTree[j]->GetEntry(i);
      is_good = is_good && sig && x > 0 && y > 0;
      posX[j] = x;
      posY[j] = y;
      posZ[j] = z;
    }
    if (is_good)
    {
      for (int j = 0; j < MMdetN; j++)
      {
        xvec[j].push_back(posX[j]);
        yvec[j].push_back(posY[j]);
        zvec[j].push_back(posZ[j]);
      }
    }
  }
  nentries = xvec[0].size();
  cout << " --> Select events = " << nentries << endl;

  double pars[MMdetN][6] = {0};
  double D_pre[MMdetN][6] = {0}, Chi_pre = 0;
  double ED[MMdetN][6] = {0}, Ed[MMdetN][6] = {0};
  for (int loop = 0;; loop++)
  {
    cout << " --> loop = " << loop << endl;
    int NChi = 0;
    double Chi2 = 0, D[MMdetN][6] = {0}, d[MMdetN][6] = {0};

    for (int entry = 0; entry < nentries; entry++)
    {
      vector<double> x0, y0, z0, xalg, yalg, zalg;
      for (int i = 0; i < MMdetN; i++)
      {
        x0.push_back(xvec[i][entry] * 0.4 - centre);
        y0.push_back(yvec[i][entry] * 0.4 - centre);
        z0.push_back(zvec[i][entry] * 0.4 - centre);
        xalg.push_back(x0[i] + pars[i][0] - y0[i] / centre * pars[i][5]);
        yalg.push_back(y0[i] + pars[i][1] + x0[i] / centre * pars[i][5]);
        zalg.push_back(z0[i] + pars[i][2] - x0[i] / centre * pars[i][4] + y0[i] / centre * pars[i][3]);
      }

      bool is_good_event = true;
      // x = K*z+B, R2 = correction index squre
      double Kx[MMdetN] = {0}, Bx[MMdetN] = {0}, Rx[MMdetN] = {0};
      double Ky[MMdetN] = {0}, By[MMdetN] = {0}, Ry[MMdetN] = {0};
      // mean
      double Mx[MMdetN] = {0}, My[MMdetN] = {0}, Mz[MMdetN] = {0};
      double Mxx[MMdetN] = {0}, Myy[MMdetN] = {0}, Mzz[MMdetN] = {0};
      double Mzx[MMdetN] = {0}, Mzy[MMdetN] = {0};
      int N = MMdetN - 1;

      for (int i = 0; i < MMdetN; i++)
      {
        for (int j = 0; j < MMdetN; j++)
        {
          if (i != j)
          {
            Mx[i] += xalg[j] / N;
            My[i] += yalg[j] / N;
            Mz[i] += zalg[j] / N;
            Mxx[i] += xalg[j] * xalg[j] / N;
            Myy[i] += yalg[j] * yalg[j] / N;
            Mzz[i] += zalg[j] * zalg[j] / N;
            Mzx[i] += zalg[j] * xalg[j] / N;
            Mzy[i] += zalg[j] * yalg[j] / N;
          }
        }
        Kx[i] = (Mzx[i] - Mz[i] * Mx[i]) / (Mzz[i] - Mz[i] * Mz[i]);
        Bx[i] = Mx[i] - Kx[i] * Mz[i];
        Rx[i] = (Mzx[i] - Mz[i] * Mx[i]) * (Mzx[i] - Mz[i] * Mx[i]) / (Mxx[i] - Mx[i] * Mx[i]) / (Mzz[i] - Mz[i] * Mz[i]);
        Ky[i] = (Mzy[i] - Mz[i] * My[i]) / (Mzz[i] - Mz[i] * Mz[i]);
        By[i] = My[i] - Ky[i] * Mz[i];
        Ry[i] = (Mzy[i] - Mz[i] * My[i]) * (Mzy[i] - Mz[i] * My[i]) / (Myy[i] - My[i] * My[i]) / (Mzz[i] - Mz[i] * Mz[i]);

        is_good_event = is_good_event && Rx[i] > alg_fit_R2_cut && Ry[i] > alg_fit_R2_cut;
        is_good_event = is_good_event && Abs(ATan(Kx[i]) / Pi() * 180) < alg_theta_cut;
        is_good_event = is_good_event && Abs(ATan(Ky[i]) / Pi() * 180) < alg_theta_cut;
        is_good_event = is_good_event && Abs(Kx[i] * zalg[i] + Bx[i] - xalg[i]) < alg_fit_res_cut;
        is_good_event = is_good_event && Abs(Ky[i] * zalg[i] + By[i] - yalg[i]) < alg_fit_res_cut;
      }
      if (!is_good_event)
        continue;

      for (int i = 0; i < MMdetN; i++)
      {
        double dx, dy; // residual
        double DKxz, DKxx, DBxz, DBxx;
        double DKyz, DKyy, DByz, DByy;

        NChi += 2;
        dx = Kx[i] * zalg[i] + Bx[i] - xalg[i];
        dy = Ky[i] * zalg[i] + By[i] - yalg[i];
        Chi2 += Power(dx, 2) + Power(dy, 2);

        for (int j = 0; j < MMdetN; j++)
        {
          if (i != j)
          {
            DKxz = ((xalg[j] - Mx[i]) / (Mzz[i] - Mz[i] * Mz[i]) - 2 * Kx[i] * (zalg[j] - Mz[i]) / (Mzz[i] - Mz[i] * Mz[i])) / N;
            DKxx = (zalg[j] - Mz[i]) / (Mzz[i] - Mz[i] * Mz[i]) / N;
            DBxz = -Kx[i] / N - DKxz * Mz[i];
            DBxx = 1. / N - DKxx * Mz[i];
            DKyz = ((yalg[j] - My[i]) / (Mzz[i] - Mz[i] * Mz[i]) - 2 * Ky[i] * (zalg[j] - Mz[i]) / (Mzz[i] - Mz[i] * Mz[i])) / N;
            DKyy = (zalg[j] - Mz[i]) / (Mzz[i] - Mz[i] * Mz[i]) / N;
            DByz = -Ky[i] / N - DKyz * Mz[i];
            DByy = 1. / N - DKyy * Mz[i];

            D[j][0] += 2 * dx * (zalg[i] * DKxx + DBxx);
            D[j][1] += 2 * dy * (zalg[i] * DKyy + DByy);
            D[j][2] += 2 * dx * (zalg[i] * DKxz + DBxz) + 2 * dy * (zalg[i] * DKyz + DByz);
            D[j][3] += yalg[j] / centre * (2 * dx * (zalg[i] * DKxz + DBxz) + 2 * dy * (zalg[i] * DKyz + DByz));
            D[j][4] += -xalg[j] / centre * (2 * dx * (zalg[i] * DKxz + DBxz) + 2 * dy * (zalg[i] * DKyz + DByz));
            D[j][5] += -yalg[j] / centre * 2 * dx * (zalg[i] * DKxx + DBxx) + xalg[j] / centre * 2 * dy * (zalg[i] * DKyy + DByy);
          }
          else
          {
            D[j][0] += -2 * dx;
            D[j][1] += -2 * dy;
            D[j][2] += 2 * dx * Kx[i] + 2 * dy * Ky[i];
            D[j][3] += yalg[j] / centre * (2 * dx * Kx[i] + 2 * dy * Ky[i]);
            D[j][4] += -xalg[j] / centre * (2 * dx * Kx[i] + 2 * dy * Ky[i]);
            D[j][5] += yalg[j] / centre * 2 * dx - xalg[j] / centre * 2 * dy;
          }
        }
      }
    } // entry loop
    cout << " --> Event number = " << NChi / MMdetN / 2 << ", Chi = " << Sqrt(Chi2 / NChi)
         << ", DeltaChi = " << Chi_pre - Sqrt(Chi2 / NChi) << endl;

    for (int i = 0; i < MMdetN; i++)
    {
      for (int j = 0; j < Npars; j++)
      {
        D[i][j] = D[i][j] / NChi * MMdetN * 2;
        if (D_pre[i][j] * D[i][j] < 0)
          Ed[i][j] = rho * Ed[i][j];
        ED[i][j] = rho * ED[i][j] + (1 - rho) * Power(D[i][j], 2);
        d[i][j] = -Sqrt(Ed[i][j] + epsilon) / Sqrt(ED[i][j] + epsilon) * D[i][j];
        Ed[i][j] = rho * Ed[i][j] + (1 - rho) * Power(d[i][j], 2);
        pars[i][j] += d[i][j];
        D_pre[i][j] = D[i][j];
        epsilon = epsilon * rho;
      }
      cout << " --> det=" << i;
      for (int j = 0; j < 6; j++)
        cout << ", " << ParName[j] << "=" << pars[i][j];
      cout << endl;
    }
    cout << endl;

    if (Abs(Chi_pre - Sqrt(Chi2 / NChi)) < res || loop > 5e3)
      break;
    else
      Chi_pre = Sqrt(Chi2 / NChi);
  }

  ofstream out(filename.c_str());
  for (int i = 0; i < MMdetN; i++)
  {
    for (int j = 0; j < 6; j++)
    {
      if (j >= 3)
        pars[i][j] = pars[i][j] / centre;
      if (Npars == 3)
        algPar3[i][j] = pars[i][j];
      if (Npars == 6)
        algPar6[i][j] = pars[i][j];
      out << pars[i][j] << " ";
    }
    out << endl;
  }
  out.close();
  cout << " --> Alignment succesfully !" << endl;
}
