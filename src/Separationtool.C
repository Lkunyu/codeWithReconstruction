#include "Separationtool.h"

using namespace std;


Separationtool::Separationtool() {};

Separationtool::~Separationtool()
{
    T0Outfile->WriteTObject(T0Tree);
    DTOFOutfile->WriteTObject(DTOFTree);
    T0Outfile->Close();
    DTOFOutfile->Close();
};

Separationtool::Separationtool(TString fileDir, int force)
{
    SetFileDir(fileDir);
}

bool Separationtool::Init()
{
    TString T0Name = fileDir + "/Combine/T0_data.root";
    TString DTOFName = fileDir + "/Combine/DTOF_data.root";
    TString InName = fileDir + "/Combine/Dtof-step2-ana.root";
    infile = new TFile(InName, "READ");
    intree = (TTree *)infile->Get("DtofData");
    intree->SetBranchAddress("event", &eventID);
    intree->SetBranchAddress("DTOF_TOA", DTOF_TOA);
    intree->SetBranchAddress("DTOF_TOT", DTOF_TOT);
    T0Outfile = new TFile(T0Name, "RECREATE");
    T0Tree = new TTree("T0data", "T0data");
    T0Tree->Branch("event", &eventID, "event/I");
    T0Tree->Branch("T0TOA", T0TOA, "T0TOA[192]/D");
    T0Tree->Branch("T0TOT", T0TOT, "T0TOT[192]/D");
    T0Tree->Branch("T0Npe", &NpeT0, "T0Npe/I");
    DTOFOutfile = new TFile(DTOFName, "RECREATE");
    DTOFTree = new TTree("DTOFdata", "DTOFdata");
    DTOFTree->Branch("event", &eventID, "event/I");
    DTOFTree->Branch("DTOFTOA", DTOFTOA, "DTOFTOA[224]/D");
    DTOFTree->Branch("DTOFTOT", DTOFTOT, "DTOFTOT[224]/D");
    DTOFTree->Branch("DTOFNpe", &NpeDTOF, "DTOFNpe/I");
    ConstructMap();

    return true;
}

void Separationtool::ConstructMap()
{
    channelMapT0.clear();
    int offsetT0[4] = {96, 160, 320, 384}; // 96,160
    // T0 map
    for (int i = 0; i < 48; i++)
    {
        channelMapT0.push_back(offsetT0[0] + i % 4 * 4 + i / 12 + i % 12 / 4 * 16); // i*4+i%12+i%12/4*16(i%12%4)*4+(i%12/4)*16+i/12
    }

    for (int i = 48; i < 96; i++)
    {
        channelMapT0.push_back(offsetT0[1] + (i - 48) % 4 * 4 + (i - 48) / 12 + (i - 48) % 12 / 4 * 16);
    }
    for (int i = 96; i < 144; i++)
    {
        channelMapT0.push_back(offsetT0[2] + (i - 96) % 4 * 4 + (i - 96) / 12 + (i - 96) % 12 / 4 * 16);
    }
    for (int i = 144; i < 192; i++)
    {
        channelMapT0.push_back(offsetT0[3] + (i - 144) % 4 * 4 + (i - 144) / 12 + (i - 144) % 12 / 4 * 16);
    }
    // for(int i=0;i<192;i++){
    //     cout<<"map "<<i<<" DTOF_TOA "<<channelMapT0.at(i)<<endl;
    // }
    // DTOF map
    int offsetDTOF = 448;
    for (int i = 0; i < 224; i++)
    {
        channelMapDTOF.push_back(offsetDTOF + i);
    }
    // for (int i = 128; i < 224; i++)
    // {
    //     channelMapDTOF.push_back(offsetDTOF[1] + i - 128);
    // }
}

void Separationtool::Loop()
{
    ifstream inlog;
    double T0offset[192];
    inlog.open("T0channelCaliIn.txt");
    // read offset
    double offsetTime;
    int counter = 0;
    while (inlog >> offsetTime)
    {
        T0offset[counter] = offsetTime;
        counter++;
    }
    // end read
    TH1D *hist[10];
    for (int i = 0; i < 10; i++)
        hist[i] = new TH1D(Form("hist_%d", i), Form("hist_%d", i), 2000, -5e4, 1e4);
    TH2I *hNpe = new TH2I("Npe", "Npe", 192, 0, 192, 100, 0, 100);
    TH1I *hHit = new TH1I("Hit", "hit", 672, 0, 672);
    TH2D *hDTOFcht[2];

    hDTOFcht[0] = new TH2D("DTOFchannel_t_pi", "DTOFchannel_t_pi", 224, 0, 224, 500, -4000, 1000);
    hDTOFcht[1] = new TH2D("DTOFchannel_t_p", "DTOFchannel_t_p", 224, 0, 224, 500, -4000, 1000);
    for (int i = 0; i < intree->GetEntries(); i++)
    {
        DrawProcessbar(i, intree->GetEntries());
        intree->GetEntry(i);
        for (int iii = 0; iii < 672; iii++)
        {
            if (DTOF_TOA[iii] < 0)
                hHit->Fill(iii);
        }
        NpeT0 = 0;
        NpeDTOF = 0;
        memset(T0TOA, 0, sizeof(T0TOA));
        memset(T0TOT, 0, sizeof(T0TOT));
        memset(DTOFTOA, 0, sizeof(DTOFTOA));
        memset(DTOFTOT, 0, sizeof(DTOFTOT));
        // T0
        //  for(int j=0;j<672;j++){
        //      if(DTOF_TOT[j]>1e-11)
        //          cout<<"channel "<<j<<endl;
        //  }
        for (int j = 0; j < 192; j++)
        {
            T0TOA[j] = DTOF_TOA[channelMapT0.at(j)];
            T0TOT[j] = DTOF_TOT[channelMapT0.at(j)];
            if (DTOF_TOA[channelMapT0.at(j)] > 1e-11 || DTOF_TOA[channelMapT0.at(j)] < -1e-11)
                NpeT0++;
            //  if (channelMapT0.at(i) < 192) // T0
            // {
            //     T0TOA[channelMapT0.at(i)] = DTOF_TOA[i];
            //     T0TOT[channelMapT0.at(i)] = DTOF_TOT[i];
            //     if (T0TOA[channelMapT0.at(i)] > 1e-1 || T0TOA[channelMapT0.at(i)] < -1e-1)
            //         NpeT0++;
            // }
            // else if (channelMapT0.at(i) >= 300) // DTOF
            // {
            //     DTOF[channelMapT0.at(i) - 300] = DTOF_TOA[i];
            //     DTOF[channelMapT0.at(i) - 300] = DTOF_TOT[i];
            //     if (DTOFTOA[channelMapT0.at(i)] > 1e-1 || DTOFTOA[channelMapT0.at(i)] < -1e-1)
            //         NpeDTOF++;
            // }
        }
        for (int j = 0; j < 224; j++)
        {
            DTOFTOA[j] = DTOF_TOA[channelMapDTOF.at(j)];
            DTOFTOT[j] = DTOF_TOT[channelMapDTOF.at(j)];
            if (DTOFTOA[j] > 1e9)
                DTOFTOA[j] = 0;
            if (DTOFTOA[j] > 1e-11 || DTOFTOA[j] < -1e-11)
                NpeDTOF++;
        }
        hNpe->Fill(NpeT0, NpeDTOF);
        // if (NpeT0 < 180)
        //     continue;
        vector<double> T0TOAs[2];
        for (int ii = 0; ii < 96; ii++)
            if (T0TOA[ii] != 0)
                T0TOAs[0].push_back(T0TOA[ii] - T0offset[ii]);
        for (int ii = 96; ii < 192; ii++)
            if (T0TOA[ii] != 0)
                T0TOAs[1].push_back(T0TOA[ii] - T0offset[ii]);
        sort(T0TOAs[0].begin(), T0TOAs[0].end());
        sort(T0TOAs[1].begin(), T0TOAs[1].end());
        double T01 = SelectRecT(T0TOAs[0], 1000, 1);
        double T02 = SelectRecT(T0TOAs[1], 1000, 1);
        hist[0]->Fill(T01 - T02);
        vector<int> DTOFNum = {10, 112};
        for (size_t iii = 0; iii < 2; iii++)
            if (DTOFTOA[DTOFNum[iii]] != 0)
            {
                if (T01 - T02 < -26600)
                    hist[iii * 2 + 1]->Fill(DTOFTOA[DTOFNum[iii]] - (T01 + T02) / 2.);
                else
                    hist[iii * 2 + 2]->Fill(DTOFTOA[DTOFNum[iii]] - (T01 + T02) / 2.);
            }
        for (int j = 0; j < 224; j++)
        {
            int k = -1;
            if (T01 - T02 < -26600)
                k = 0;
            else
                k = 1;

            if (DTOFTOA[j] != 0)
                hDTOFcht[k]->Fill(j, DTOFTOA[j] - (T01 + T02) / 2.0);
        }
        T0Tree->Fill();
        DTOFTree->Fill();
    }
    for(int i=0;i<2;i++)
        hDTOFcht[i]->Scale(1./hDTOFcht[i]->Integral());
    for (int i = 0; i < 5; i++)
        T0Outfile->WriteTObject(hist[i]);
    T0Outfile->WriteTObject(hNpe);
    T0Outfile->WriteTObject(hHit);
    for (int i = 0; i < 2; i++)
    {
        DTOFOutfile->WriteTObject(hDTOFcht[i]);
    }

    TCanvas* DTOFChannelT=new TCanvas("c","");
    DTOFChannelT->cd();
    
    hDTOFcht[0]->SetLineColor(kRed);
    hDTOFcht[0]->Draw("box");
    hDTOFcht[1]->SetLineColor(kBlue);
    hDTOFcht[1]->Draw("boxsame");
    
    DTOFOutfile->WriteTObject(DTOFChannelT);

    cout << "finish" << endl;
}
