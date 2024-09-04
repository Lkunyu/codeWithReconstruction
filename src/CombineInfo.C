#include "CombineInfo.h"

CombineInfo::CombineInfo(TString fTrackName, TString fT0Name, TString fDTOFName, TString fComName, int f_force)
{
    trackName = fTrackName;
    T0Name = fT0Name;
    DTOFName = fDTOFName;
    comName = fComName;
    force = f_force;
}
CombineInfo::~CombineInfo()
{
}

bool CombineInfo::Init()
{
    ftrackFile = new TFile(trackName, "read");
    if (!ftrackFile->IsOpen())
    {
        cout << " ERROR! " << trackName << " cant open!" << endl;
        return false;
    }
    ftrackTree=(TTree *)ftrackFile->Get("Track");
    if (ftrackTree == 0)
    {
        cout << " ERROR! Track Tree not exist in " << trackName << endl;
        return false;
    }
    ftrackTree->SetBranchAddress("event", &event, &b_event_track);
    ftrackTree->SetBranchAddress("MMx", MMx, &b_MMx);
    ftrackTree->SetBranchAddress("MMy", MMy, &b_MMy);
    ftrackTree->SetBranchAddress("MMz", MMz, &b_MMz);
    ftrackTree->SetBranchAddress("MMsigx", MMsigx, &b_MMsigx);
    ftrackTree->SetBranchAddress("MMsigy", MMsigy, &b_MMsigy);
    ftrackTree->SetBranchAddress("XfitPars", XfitPars, &b_XfitPars);
    ftrackTree->SetBranchAddress("YfitPars", YfitPars, &b_YfitPars);
    ftrackTree->SetBranchAddress("useflag",&useflag);
    fT0File = new TFile(T0Name, "read");
    if (!fT0File->IsOpen())
    {
        cout << " ERROR! " << T0Name << " cant open!" << endl;
        return false;
    }
    fT0Tree=(TTree *)fT0File->Get("T0data");
    if (fT0Tree == 0)
    {
        cout << " ERROR! T0 Tree not exist in " << T0Name << endl;
        return false;
    }
    fT0Tree->SetBranchAddress("event", &event);
    fT0Tree->SetBranchAddress("T0TOA", T0_TOA);
    fT0Tree->SetBranchAddress("T0TOT", T0_TOT);
    fT0Tree->SetBranchAddress("T0Npe", &T0_Npe);

    fDTOFFile = new TFile(DTOFName, "read");
    if (!fDTOFFile->IsOpen())
    {
        cout << " ERROR! " << DTOFName << " cant open!" << endl;
        return false;
    }
    fDTOFTree=(TTree *)fDTOFFile->Get("DTOFdata");
    if (fDTOFTree == 0)
    {
        cout << " ERROR! DTOF Tree not exist in " << DTOFName << endl;
        return false;
    }
    fDTOFTree->SetBranchAddress("event", &event);
    fDTOFTree->SetBranchAddress("DTOFTOA", DTOF_TOA);
    fDTOFTree->SetBranchAddress("DTOFTOT", DTOF_TOT);
    fDTOFTree->SetBranchAddress("DTOFNpe", &DTOF_Npe);

    if (force != 1)
    {
        FileStat_t fStat;
        gSystem->GetPathInfo(comName, fStat);
        if (fStat.fSize != 0)
        {
            cout << " ERROR! " << comName << " is exist!" << endl;
            return false;
        }
    }

    fcomFile = new TFile(comName, "recreate");
    if (!fcomFile->IsOpen())
    {
        cout << "ERROR! " << comName << " cant open!" << endl;
        return false;
    }
    fcomTree = new TTree("ComData", "ComData");
    fcomTree->Branch("event", &event, "event/I");
    fcomTree->Branch("tracktheta", &tracktheta, "tracktheta/D");
    fcomTree->Branch("trackTOF", &trackTOF, "trackTOF/D");
    fcomTree->Branch("trackDir", trackDir, "trackDir[3]/D");
    fcomTree->Branch("T0_Npe", &T0_Npe, "T0_Npe/I");
    fcomTree->Branch("T0_TOA", T0_TOA, "T0_TOA[192]/D");
    fcomTree->Branch("T0_TOT", T0_TOT, "T0_TOT[192]/D");
    fcomTree->Branch("T0_trackPos", T0_trackPos, "T0_trackPos[4]/D");
    fcomTree->Branch("DTOF_Npe", &DTOF_Npe, "DTOF_Npe/I");
    fcomTree->Branch("DTOF_TOA", DTOF_TOA, "DTOF_TOA[224]/D");
    fcomTree->Branch("DTOF_TOT", DTOF_TOT, "DTOF_TOT[224]/D");
    fcomTree->Branch("DTOF_trackPos", DTOF_trackPos, "DTOF_trackPos[2]/D");

    return true;
}

void CombineInfo::Loop()
{
    ofstream op = LogFile(comName);
    TCanvas *cTmp = new TCanvas("tmpcom", "tmpcom");
    //  TH1F *hT0tot[4];
    //  TH1F *hT0temp[4];
    //   for(int i=0; i<4; i++)
    //   {
    //     hT0tot[i] = new TH1F(Form("T0%d_TOT",i),Form("T0%d_TOT",i),400,200,1500);
    //     hT0tot[i]->GetXaxis()->SetTitle("TOT [ps]");
    //     hT0tot[i]->GetYaxis()->SetTitle("Counts");
    //     hT0temp[i] = new TH1F(Form("T0%d_FPGAtemp",i),Form("T0%d_FPGAtemp",i),400,25,45);
    //     hT0temp[i]->GetXaxis()->SetTitle("FPGA temperature [Celsius]");
    //     hT0temp[i]->GetYaxis()->SetTitle("Counts");
    //   }
    TH1I *hT0Eff[4];
    for (int i = 0; i < 4; i++)
    {
        hT0Eff[i] = new TH1I(Form("T0%d_%d", i / 2, i % 2), Form("T0%d_%d", i / 2, i % 2), 48, 0, 48);
        hT0Eff[i]->GetXaxis()->SetTitle("Number of fired channels");
        hT0Eff[i]->GetYaxis()->SetTitle("Counts");
        hT0Eff[i]->SetLineWidth(2);
    }

    TH1I *hT0PMTEff = new TH1I("T0_PMT_Eff", "T0_PMT_Eff", 192, 0, 192);
    hT0PMTEff->GetXaxis()->SetTitle("Fired PMT ID");
    hT0PMTEff->GetYaxis()->SetTitle("Counts");
    hT0PMTEff->SetLineWidth(2);

    TH1F *hDTOFtot[224];
    for (int i = 0; i < 224; i++)
    {
        hDTOFtot[i] = new TH1F(Form("DTOF_PMT%02dch%02d_TOT", i / 16, i % 16), Form("DTOF_PMT%02dch%02d_TOT", i / 16, i % 16), 400, 200, 2000);
        hDTOFtot[i]->GetXaxis()->SetTitle("TOT [ps]");
        hDTOFtot[i]->GetYaxis()->SetTitle("Counts");
    }
    TH1F *hDTOFtemp[7];
    for (int i = 0; i < 7; i++)
    {
        hDTOFtemp[i] = new TH1F(Form("DTOF_board%d_FPGAtemp", i + 1), Form("DTOF_board%d_FPGAtemp", i + 1), 400, 30, 70);
        hDTOFtemp[i]->GetXaxis()->SetTitle("FPGA temperature [Celsius]");
        hDTOFtemp[i]->GetYaxis()->SetTitle("Counts");
    }
    TH1I *hDTOFnpe = new TH1I("DTOF_Npe", "DTOF_Npe", 150, 0, 150);
    hDTOFnpe->GetXaxis()->SetTitle("Number of photoelectrons");
    hDTOFnpe->GetYaxis()->SetTitle("Counts");
    hDTOFnpe->SetLineWidth(2);
    TH1I *hDTOFchEff = new TH1I("DTOF_ch_Eff", "DTOF_ch_Eff", 224, 0, 224);
    hDTOFchEff->GetXaxis()->SetTitle("Fired channel ID");
    hDTOFchEff->GetYaxis()->SetTitle("Counts");
    hDTOFchEff->SetLineWidth(2);
    TH2I *hDTOFPMTEff = new TH2I("DTOF_PMT_Eff", "DTOF_PMT_Eff", 7, 0, 7, 16, 0, 16);
    hDTOFPMTEff->GetXaxis()->SetTitle("PMT ID");
    hDTOFPMTEff->GetYaxis()->SetTitle("Number of photoelectrons");

    TH2F *hT0trackHit[2];
    for (int i = 0; i < 2; i++)
    {
        hT0trackHit[i] = new TH2F(Form("T0_%dtrackHit", i), Form("T0_%dtrackHit", i), 100, -100, 100, 100, -100, 100);
        hT0trackHit[i]->GetXaxis()->SetTitle("X [mm]");
        hT0trackHit[i]->GetYaxis()->SetTitle("Y [mm]");
    }

    TH2F *hDTOFtrackHit = new TH2F("DTOFtrackHit", "DTOFtrackHit", 100, -100, 100, 100, -100, 100); // using its own coordinnate,need to be recalculated
    hDTOFtrackHit->GetXaxis()->SetTitle("X [mm]");
    hDTOFtrackHit->GetYaxis()->SetTitle("Y [mm]");
    TH1F *htrackTheta = new TH1F("trackTheta", "trackTheta", 200, 0, 15);
    htrackTheta->GetXaxis()->SetTitle("polar angle for track [degree]");
    htrackTheta->GetYaxis()->SetTitle("Counts");
    TH1F *htrackTOF = new TH1F("trackTOF", "trackTOF", 200, 1525, 1565);
    htrackTOF->GetXaxis()->SetTitle("TOF for track [ps]");
    htrackTOF->GetYaxis()->SetTitle("Counts");

    ///////////
    int T0Entry = fT0Tree->GetEntries();
    int trackEntry = ftrackTree->GetEntries();
    int DTOFEntry = fDTOFTree->GetEntries();

    fT0Tree->GetEntry(0);
    int start = event;
    fT0Tree->GetEntry(T0Entry - 1);
    int end = event;
    int N_combine[3] = {0}; // track, track&DTOF, track&DTOF&T0

    /////////// begin to loop
    int eventID = 0;
    int eventT0, eventDTOF = 0;
    int pos = 0;
    int T0FiredChannels[4];
    int trackerX=0;
    int trackerY=0;
    bool trackerXY;//1 means all 0 means cannot found

    for (int i = 0; i < trackEntry; i++)
    {
        for (int i = 0; i < 4; i++)
        {
            T0FiredChannels[i] = 0;
        }
        DrawProcessbar(i, trackEntry);
        InitData();
        ftrackTree->GetEntry(i);
        eventID = event;
        //cout<<"ID from Tracker "<<eventID<<endl;
        for (int j = pos; j < T0Entry; j++)
        {
            fT0Tree->GetEntry(j);
            fDTOFTree->GetEntry(j);
            if (event >= eventID)
            {
                pos = j;
                break;
            }
            
            // if (event >= eventID)
            // {
            //     pos = j;
            //     break;
            // }
        }
        
            trackerX=(MMsigx[0]+MMsigx[1]+MMsigx[2]+MMsigx[3]);
            trackerY=(MMsigy[0]+MMsigy[1]+MMsigy[2]+MMsigy[3]);
            trackerXY=(trackerX>=2)&&(trackerY>=2);

        if ((event == eventID)&&useflag)
        {
            RecTrackHit();
            fcomTree->Fill();

            N_combine[0]++;
            if (DTOF_Npe >= 5)
                N_combine[1]++;
            if (DTOF_Npe >= 5 && T0_Npe > 160)
                N_combine[2]++;

            hT0trackHit[0]->Fill(T0_trackPos[0], T0_trackPos[1]);
            hT0trackHit[1]->Fill(T0_trackPos[2], T0_trackPos[3]);
            hDTOFtrackHit->Fill(DTOF_trackPos[0], DTOF_trackPos[1]);
            htrackTheta->Fill(tracktheta / TMath::Pi() * 180);
            htrackTOF->Fill(trackTOF);
            for (int j = 0; j < 192; j++)
            {
                if (T0_TOA[j] == 0)
                    continue;
                // hT0tot[j]->Fill(T0_TOT[j]);
                // hT0temp[j]->Fill(T0_temp[j]);
                hT0PMTEff->Fill(j);
                T0FiredChannels[j / 48]++;
            }
            for (int j = 0; j < 4; j++)
            {
                hT0Eff[j]->Fill(T0FiredChannels[j]);
            }

            hDTOFnpe->Fill(DTOF_Npe);
            for (int j = 0; j < 224; j++)
            {
                if (DTOF_TOA[j] == 0)
                    continue;
                hDTOFtot[j]->Fill(DTOF_TOT[j]);
                hDTOFtemp[j / 32]->Fill(DTOF_temp[j]);
                hDTOFchEff->Fill(j);
            }
            for (int j = 0; j < 7; j++)
            {
                int npe = 0;
                for (int k = 0; k < 16; k++)
                    if (DTOF_TOA[j * 16 + k] != 0)
                        npe++;
                hDTOFPMTEff->Fill(j, npe);
            }
        }
    }
    ftrackFile->Close();
    fT0File->Close();
    fDTOFFile->Close();
    fcomFile->WriteTObject(fcomTree);

    cout << " --> total triggers: " << T0Entry << ", start from " << start << " to " << end << endl;
    cout << " --> total valid tracks: " << N_combine[0] << ", ratio: " << N_combine[0] / double(T0Entry) << endl;
    cout << " --> track&DTOF(Npe>=5) events: " << N_combine[1] << ", ratio: " << N_combine[1] / double(T0Entry) << endl;
    cout << " --> track&DTOF&T0(Npe>1) events: " << N_combine[2] << ", ratio: " << N_combine[2] / double(T0Entry) << endl;
    cout << " --> DTOF efficiency (Npe>=5): " << N_combine[1] / double(N_combine[0]) << endl;
    op << " --> total triggers: " << T0Entry << ", start from " << start << " to " << end << endl;
    op << " --> total valid tracks: " << trackEntry << ", ratio: " << trackEntry / double(T0Entry) << endl;
    op << " --> track&DTOF(Npe>=5) events: " << N_combine[1] << ", ratio: " << N_combine[1] / double(T0Entry) << endl;
    op << " --> track&DTOF&T0(Npe>1) events: " << N_combine[2] << ", ratio: " << N_combine[2] / double(T0Entry) << endl;
    op << " --> DTOF efficiency (Npe>=5): " << N_combine[1] / double(N_combine[0]) << endl;

    // Draw hit information
    TLegend *leg;
    TLatex *la;
    int maximum_counts = 0;
    TString latexbuff;

    // Draw track hit information
    TCanvas *cTrack = new TCanvas("TrackHit", "TrackHit");
    cTrack->Divide(3, 2);
    cTrack->cd(1);
    htrackTheta->Draw();
    cTrack->cd(2);
    htrackTOF->Draw();
    cTrack->cd(3);
    hT0trackHit[0]->Draw("COL");
    cTrack->cd(4);
    hT0trackHit[1]->Draw("COL");
    cTrack->cd(5);
    hDTOFtrackHit->Draw("COL");
    fcomFile->WriteTObject(cTrack);

    // Draw T0 hit information
    TCanvas *cT0 = new TCanvas("T0hit", "T0hit");
    cT0->Divide(2, 2);

    cT0->cd(1);
    //   leg = new TLegend(0.2,0.5,0.3,0.85);
    //   maximum_counts = 0;
    //   for(int i=0; i<4; i++)
    //     if(maximum_counts < hT0tot[i]->GetMaximum()) maximum_counts = hT0tot[i]->GetMaximum();
    //   for(int i=0; i<4; i++)
    //   {
    //     hT0tot[i]->SetLineColor(color[i]);
    //     hT0tot[i]->GetYaxis()->SetRangeUser(0,maximum_counts*1.3);
    //     if(i == 0) hT0tot[i]-> Draw();
    //     else hT0tot[i]->Draw("same");
    //     leg->AddEntry(hT0tot[i],Form("T0%d",i),"lpf");
    //   }
    //   leg->Draw("same");

    //   cT0->cd(2);
    //   leg = new TLegend(0.2,0.5,0.3,0.85);
    //   maximum_counts = 0;
    //   for(int i=0; i<4; i++)
    //     if(maximum_counts < hT0temp[i]->GetMaximum()) maximum_counts = hT0temp[i]->GetMaximum();
    //   for(int i=0; i<4; i++)
    //   {
    //     hT0temp[i]->SetLineColor(color[i]);
    //     hT0temp[i]->GetYaxis()->SetRangeUser(0,maximum_counts*1.3);
    //     if(i == 0) hT0temp[i]-> Draw();
    //     else hT0temp[i]->Draw("same");
    //     leg->AddEntry(hT0temp[i],Form("T0%d",i),"lpf");
    //   }
    //   leg->Draw("same");

    //   cT0->cd(3);
    for (int i = 0; i < 4; i++)
    {
        cT0->cd(i);
        hT0Eff[i]->GetYaxis()->SetRangeUser(0, hT0Eff[i]->GetMaximum() * 1.1);
        hT0Eff[i]->Draw();
    }

    cT0->cd(4);
    hT0PMTEff->GetYaxis()->SetRangeUser(0, hT0PMTEff->GetMaximum() * 1.2);
    hT0PMTEff->Draw();
    // double T0PMTEff[4];
    // for (int i = 0; i < 4; i++)
    // {
    //     T0PMTEff[i] = hT0PMTEff->GetBinContent(hT0PMTEff->FindBin(i)) / N_combine[0];
    //     latexbuff = Form("PMTID=%d, Ratio=%.2f%%", i, T0PMTEff[i] * 100);
    //     cout << " --> T0 " << latexbuff << endl;
    //     op << " --> T0 " << latexbuff << endl;
    //     la = CreatLatex(latexbuff, 0.25, 0.7 - 0.07 * i, 42);
    //     la->Draw("same");
    // }

    fcomFile->WriteTObject(cT0);

    // Draw DTOF hit information
    TCanvas *cDTOFtot = new TCanvas("DTOFtot", "DTOFtot");
    cDTOFtot->Divide(2, 4);
    for (int i = 0; i < 7; i++)
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
    fcomFile->WriteTObject(cDTOFtot);

    TCanvas *cDTOF = new TCanvas("DTOFhit", "DTOFhit");
    cDTOF->Divide(2, 2);

    cDTOF->cd(1);
    hDTOFPMTEff->Draw("COLZ");

    cDTOF->cd(2);
    leg = new TLegend(0.15, 0.15, 0.35, 0.85);
    maximum_counts = 0;
    for (int i = 0; i < 7; i++)
        if (maximum_counts < hDTOFtemp[i]->GetMaximum())
            maximum_counts = hDTOFtemp[i]->GetMaximum();
    for (int i = 0; i < 7; i++)
    {
        hDTOFtemp[i]->SetLineColor(color[i]);
        hDTOFtemp[i]->GetYaxis()->SetRangeUser(0, maximum_counts * 1.3);
        if (i == 0)
            hDTOFtemp[i]->Draw();
        else
            hDTOFtemp[i]->Draw("same");
        leg->AddEntry(hDTOFtemp[i], Form("DTOF_board%d", i + 1), "lpf");
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

    fcomFile->WriteTObject(cDTOF);

    fcomFile->Close();
    cout << " --> Combined data has been stored to " << comName << endl;
    op << " --> Combined data has been stored to " << comName << endl;
    op.close();
}

void CombineInfo::RecTrackHit()
{
    DTOF_trackPos[0] = XfitPars[0] * Zpos[6] + XfitPars[1];
    DTOF_trackPos[1] = YfitPars[0] * Zpos[6] + YfitPars[1];
    T0_trackPos[0] = XfitPars[0] * Zpos[4] + XfitPars[1];
    T0_trackPos[1] = YfitPars[0] * Zpos[4] + YfitPars[1];
    T0_trackPos[2] = XfitPars[0] * Zpos[5] + XfitPars[1];
    T0_trackPos[3] = YfitPars[0] * Zpos[5] + YfitPars[1];
    TVector3 trackL(DTOF_trackPos[0] - T0_trackPos[0], DTOF_trackPos[1] - T0_trackPos[1], Zpos[6] - Zpos[4]);
    trackDir[0] = trackL.x() / trackL.Mag();
    trackDir[1] = trackL.y() / trackL.Mag();
    trackDir[2] = trackL.z() / trackL.Mag();
    tracktheta = TMath::Pi() - trackL.Theta();
    trackTOF = trackL.Mag() / BETA / TMath::C() * 1e9; // ps
}

void CombineInfo::InitData()
{
    tracktheta = 0;
    trackTOF = 0;
    DTOF_trackPos[0]=0;
    DTOF_trackPos[1]=0;
    for(int i=0;i<4;i++){
        T0_trackPos[i]=0;
    }
    for(int i=0;i<3;i++){
        trackDir[i]=0;
    }
    // memset(DTOF_trackPos, 0, sizeof(DTOF_trackPos));
    // memset(T0_trackPos, 0, sizeof(T0_trackPos));
    // memset(trackDir, 0, sizeof(trackDir));
}
