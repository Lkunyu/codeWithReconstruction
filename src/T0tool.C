#include "T0tool.h"

T0tool::T0tool() {};

T0tool::T0tool(TString fileDirectory, int force)
{
    SetFileDir(fileDirectory);
};
T0tool::~T0tool()
{
    T0dataOut->WriteTObject(T0dataOutTree);
    T0dataOut->Close();
}
bool T0tool::Init()
{
    TString T0dataInName = fileDir + "/Combine/T0_data.root";
    T0dataIn = new TFile(T0dataInName, "READ");
    T0dataTree = (TTree *)T0dataIn->Get("T0data");
    T0dataTree->SetBranchAddress("event", &event);
    T0dataTree->SetBranchAddress("T0TOA", T0TOA);
    T0dataTree->SetBranchAddress("T0TOT", T0TOT);
    T0dataTree->SetBranchAddress("T0Npe", &Npe);
    TString T0dataOutName = fileDir + "/Combine/T0_Result_data.root";
    T0dataOut = new TFile(T0dataOutName, "RECREATE");
    T0dataOutTree = new TTree("T0Result", "T0Result");
    T0dataOutTree->Branch("event", &event, "event/I");
    T0dataOutTree->Branch("particleType", &particletype, "particleType/I"); // particlemass
    T0dataOutTree->Branch("T01Time", &time0, "T01Time/D");
    T0dataOutTree->Branch("T02Time", &time1, "T02Time/D");
    fitgaus = new TF1("fitting curve", "gaus", -3000, 3000, "Q");
    return true;
}

void T0tool::Loop()
{
    ifstream inlog;
    double offset[192];
    inlog.open("T0channelCaliIn.txt");
    // read offset
    double offsetTime;
    int counter = 0;
    while (inlog >> offsetTime)
    {
        offset[counter] = offsetTime;
        counter++;
    }
    // end read
    vector<double> T01_plate1_vec, T01_plate2_vec, T02_plate1_vec, T02_plate2_vec;
    vector<double> T0left[4], T0right[4];
    double timet11 = 0, timet12 = 0, timet21 = 0, timet22 = 0;
    double TL[4], TR[4];
    TH1D *histTime = new TH1D("T2-T1", "", 5000, 10000, 40000);
    TH1D *deltaTLR[4];
    for (int i = 0; i < 4; i++)
        deltaTLR[i] = new TH1D(Form("L-R_%i", i), Form("L-R_%i", i), 500, -1000, 1000);
    cout<<"-->Process T0 Result file..."<<endl;
    for (int i = 0; i < T0dataTree->GetEntries(); i++)
    {
        T0dataTree->GetEntry(i);
        DrawProcessbar(i, T0dataTree->GetEntries());
        if (Npe < 160)
        {// 96 is a half
            ResetData();
            T0dataOutTree->Fill();
            continue;
        } 
        
        T01_plate1_vec.clear();
        T01_plate2_vec.clear();
        T02_plate1_vec.clear();
        T02_plate2_vec.clear();
        for(int j=0;j<4;j++){
            T0left[j].clear();
            T0right[j].clear();
        }
        time0 = 0;
        time1 = 0;
        // cout<<"event begin"<<endl;
        for (int j = 0; j < 192; j++)
        {
            // cout<<T0TOA[j]<<" "<<j<<endl;
            if ((T0TOA[j] < 1e-11 && T0TOA[j] > -1e-11))
                continue;
            switch (j / 24)
            {
            case 0:
                T01_plate1_vec.push_back(T0TOA[j] - offset[j]);
                T0left[0].push_back(T0TOA[j] - offset[j]);
                break;
            case 1:
                T01_plate2_vec.push_back(T0TOA[j] - offset[j]);
                T0left[1].push_back(T0TOA[j] - offset[j]);
                break;
            case 2:
                T01_plate1_vec.push_back(T0TOA[j] - offset[j]);
                T0right[0].push_back(T0TOA[j] - offset[j]);
                break;
            case 3:
                T01_plate2_vec.push_back(T0TOA[j] - offset[j]);
                T0right[1].push_back(T0TOA[j] - offset[j]);
                break;

            case 4:
                T02_plate1_vec.push_back(T0TOA[j] - offset[j]);
                T0left[2].push_back(T0TOA[j] - offset[j]);
                break;
            case 5:
                T02_plate2_vec.push_back(T0TOA[j] - offset[j]);
                T0left[3].push_back(T0TOA[j] - offset[j]);
                break;
            case 6:
                T02_plate1_vec.push_back(T0TOA[j] - offset[j]);
                T0right[2].push_back(T0TOA[j] - offset[j]);
                break;
            case 7:
                T02_plate2_vec.push_back(T0TOA[j] - offset[j]);
                T0right[3].push_back(T0TOA[j] - offset[j]);
                break;

            default:
                break;
            }
        }
        // vector process
        if (T01_plate1_vec.size() != 0)
        {
            SortRecT(T01_plate1_vec);
            SelectRecT(T01_plate1_vec, 1500);
        }
        if (T01_plate2_vec.size() != 0)
        {
            SortRecT(T01_plate2_vec);
            SelectRecT(T01_plate2_vec, 1500);
        }
        if (T02_plate1_vec.size() != 0)
        {
            SortRecT(T02_plate1_vec);
            SelectRecT(T02_plate1_vec, 1500);
        }
        if (T02_plate2_vec.size() != 0)
        {
            SortRecT(T02_plate2_vec);
            SelectRecT(T02_plate2_vec, 1500);
        }

        // for(int k=0;k<T01_plate1_vec.size();k++){
        //     cout<<T01_plate1_vec.at(k)<<endl;
        // }

        timet11 = CalVectorMean(T01_plate1_vec, 5, 1);
        timet12 = CalVectorMean(T01_plate2_vec, 5, 1);
        timet21 = CalVectorMean(T02_plate1_vec, 5, 1);
        timet22 = CalVectorMean(T02_plate2_vec, 5, 1);
        // cout<<timet22-timet11<<endl;
        time0 = (timet11 + timet12) / 2;
        time1 = (timet21 + timet22) / 2;
        histTime->Fill(-(timet11 + timet12) / 2 + (timet21 + timet22) / 2);
        particletype = 0;
        T0dataOutTree->Fill();
        ////left and right
        for (int ii = 0; ii < 4; ii++)
        {
            if (T0left[ii].size() != 0)
            {
                SortRecT(T0left[ii]);
                SelectRecT(T0left[ii], 1500);
                TL[ii] = CalVectorMean(T0left[ii]);
            }
            if (T0right[ii].size() != 0)
            {
                SortRecT(T0right[ii]);
                SelectRecT(T0right[ii], 1500);
                TR[ii] = CalVectorMean(T0right[ii]);
            }
        }
        for (int ii = 0; ii < 4; ii++)
            deltaTLR[ii]->Fill(TL[ii] - TR[ii]);
        /// left and right

        // if (-(timet11 + timet12) / 2 + (timet21 + timet22) / 2 > 28000 || -(timet11 + timet12) / 2 + (timet21 + timet22) / 2 < 24000)
        // {
        //     cout << "event begin" << endl;
        //     for (int j = 0; j < 192; j++)
        //     {
        //         cout << T0TOA[j] << " " << j << endl;
        //     }
        // }
    }
    T0dataOut->WriteTObject(histTime);
    for (int i = 0; i < 4; i++)
    {
        T0dataOut->WriteTObject(deltaTLR[i]);
    }
}

void T0tool::Calibration()
{
    ofstream outlog;
    outlog.open("T0channelCali.txt"); // from channel 0 to channel 192
    ifstream inlog;
    double offset[192];
    inlog.open("T0channelCaliIn.txt");
    // read offset
    double offsetTime;
    int counter = 0;
    while (inlog >> offsetTime)
    {
        offset[counter] = offsetTime;
        counter++;
    }
    // end read
    vector<double> T01_plate1_vec, T01_plate2_vec, T02_plate1_vec, T02_plate2_vec;
    double timet11 = 0, timet12 = 0, timet21 = 0, timet22 = 0;
    TString fileName = fileDir + "/Combine/CaliT0.root";
    TFile *fileCali = new TFile(fileName, "RECREATE");
    TH1D *histT01[96];
    TH1D *histT02[96];
    TH1D *histTime = new TH1D("DeltaT", "", 5000, 0, 1e5);
    for (int i = 0; i < 96; i++)
    {
        histT01[i] = new TH1D(Form("channel_%d_T01", i), Form("channel_%d_T01", i), 500, -5000, 5000);
    }
    for (int i = 0; i < 96; i++)
    {
        histT02[i] = new TH1D(Form("channel_%d_T02", i), Form("channel_%d_T02", i), 500, -5000, 5000);
    }
    int EntryTotal = 0;
    int EntryEffect = 0;
    cout << "-->Calibration Reading and processing..." << endl;
    for (int i = 0; i < T0dataTree->GetEntries(); i++)
    {
        T0dataTree->GetEntry(i);
        DrawProcessbar(i, T0dataTree->GetEntries());
        if (Npe < 180)// 96 is a half
        {
            //empty package
            continue;
        } 
        T01_plate1_vec.clear();
        T01_plate2_vec.clear();
        T02_plate1_vec.clear();
        T02_plate2_vec.clear();
        time0 = 0;
        time1 = 0;
        // cout<<"event begin"<<endl;
        for (int j = 0; j < 192; j++)
        {
            // cout<<T0TOA[j]<<" "<<j<<endl;
            if ((T0TOA[j] < 1e-11 && T0TOA[j] > -1e-11))
                continue;
            switch (j / 24)
            {
            case 0:
                T01_plate1_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 1:
                T01_plate2_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 2:
                T01_plate1_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 3:
                T01_plate2_vec.push_back(T0TOA[j] - offset[j]);
                break;

            case 4:
                T02_plate1_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 5:
                T02_plate2_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 6:
                T02_plate1_vec.push_back(T0TOA[j] - offset[j]);
                break;
            case 7:
                T02_plate2_vec.push_back(T0TOA[j] - offset[j]);
                break;

            default:
                break;
            }
        }
        // vector process
        if (T01_plate1_vec.size() != 0)
        {
            SortRecT(T01_plate1_vec);
            SelectRecT(T01_plate1_vec, 1500);
        }
        if (T01_plate2_vec.size() != 0)
        {
            SortRecT(T01_plate2_vec);
            SelectRecT(T01_plate2_vec, 1500);
        }
        if (T02_plate1_vec.size() != 0)
        {
            SortRecT(T02_plate1_vec);
            SelectRecT(T02_plate1_vec, 1500);
        }
        if (T02_plate2_vec.size() != 0)
        {
            SortRecT(T02_plate2_vec);
            SelectRecT(T02_plate2_vec, 1500);
        }

        // for(int k=0;k<T01_plate1_vec.size();k++){
        //     cout<<T01_plate1_vec.at(k)<<endl;
        // }

        timet11 = CalVectorMean(T01_plate1_vec, 5, 1);
        timet12 = CalVectorMean(T01_plate2_vec, 5, 1);
        timet21 = CalVectorMean(T02_plate1_vec, 5, 1);
        timet22 = CalVectorMean(T02_plate2_vec, 5, 1);
        // cout<<timet22-timet11<<endl;

        histTime->Fill(-(timet11 + timet12) / 2 + (timet21 + timet22) / 2);
        EntryEffect++;

        // if (-(timet11 + timet12) / 2 + (timet21 + timet22) / 2 > 28000 || -(timet11 + timet12) / 2 + (timet21 + timet22) / 2 < 24000)
        // {
        //     cout << "event begin" << endl;
        //     for (int j = 0; j < 192; j++)
        //     {
        //         cout << T0TOA[j] << " " << j << endl;
        //     }
        // }

        for (int j = 0; j < 96; j++)
        {
            switch (j / 24)
            {
            case 0:
                if (T0TOA[j] > 1e-11 || T0TOA[j] < -1e-11)
                    histT01[j]->Fill(T0TOA[j] - offset[j] - timet12);
                // cout<<"deltaT "<<T0TOA[j]-timet12<<endl;
                if (T0TOA[j + 96] > 1e-11 || T0TOA[j + 96] < -1e-11)
                    histT02[j]->Fill(T0TOA[j + 96] - offset[j + 96] - timet22);
                break;
            case 1:
                if (T0TOA[j] > 1e-11 || T0TOA[j] < -1e-11)
                    histT01[j]->Fill(T0TOA[j] - offset[j] - timet11);
                if (T0TOA[j + 96] > 1e-11 || T0TOA[j + 96] < -1e-11)
                    histT02[j]->Fill(T0TOA[j + 96] - offset[j + 96] - timet21);
                break;
            case 2:
                if (T0TOA[j] > 1e-11 || T0TOA[j] < -1e-11)
                    histT01[j]->Fill(T0TOA[j] - offset[j] - timet12);
                if (T0TOA[j + 96] > 1e-11 || T0TOA[j + 96] < -1e-11)
                    histT02[j]->Fill(T0TOA[j + 96] - offset[j + 96] - timet22);
                break;
            case 3:
                if (T0TOA[j] > 1e-11 || T0TOA[j] < -1e-11)
                    histT01[j]->Fill(T0TOA[j] - offset[j] - timet11);
                if (T0TOA[j + 96] > 1e-11 || T0TOA[j + 96] < -1e-11)
                    histT02[j]->Fill(T0TOA[j + 96] - offset[j + 96] - timet21);
                break;
            default:
                break;
            }
        }
    }
    histTime->Fit(fitgaus, "Q", "", 25000, 27000);
    fileCali->WriteTObject(histTime);
    for (int i = 0; i < 96; i++)
    {
        histT01[i]->Fit(fitgaus,"Q");
        fileCali->WriteTObject(histT01[i]);
        outlog << fitgaus->GetParameter(1) << endl;
    }
    // outlog << " " << endl;
    for (int i = 0; i < 96; i++)
    {
        histT02[i]->Fit(fitgaus,"Q");
        fileCali->WriteTObject(histT02[i]);
        outlog << fitgaus->GetParameter(1) << endl;
    }

    cout << "total " << T0dataTree->GetEntries() << " effect" << EntryEffect << endl;
    fileCali->Close();
}

void T0tool::ResetData()
{
  time0=0;
  time1=0;
  particletype=0;
}