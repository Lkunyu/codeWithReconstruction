#include "Rectool.h"

Rectool::Rectool(TString fileDirectory, int f_force)
{
    fileDir = fileDirectory;
    force = f_force;
}
Rectool::~Rectool()
{
    fCombine->Close();
    fRecT0->Close();
    fCaliRec->Close();
    fRecDTOF->Close();
    fSample->Close();
}
bool Rectool::Init()
{ //----------------Init pars
    k[0] = -14.82;
    b[0] = -160;
    k[1] = -14.72;
    b[1] = -161.9;
    k[2] = -15.59;
    b[2] = -41.57;
    k[3] = -15.58;
    b[3] = -33.95;
    // for(int i=0;i<4;i++){
    //     k[i]=0;
    //     b[i]=0;
    // }
    fitline = new TF1("line", "[0]*x+[1]");
    //----------------read offset--------------------------//
    ifstream inlog;
    inlog.open("T0Cali1.txt");
    // read offset
    double offsetTime;
    int counter = 0;
    while (inlog >> offsetTime)
    {
        offset[counter] = offsetTime;
        counter++;
    }
    if (counter > 192)
        cout << " --> offset file Wrong! please check..." << endl;
    ifstream inposlog;
    inposlog.open("calibration.txt");
    if (!inposlog.is_open())
        Calibration();
    else
    {
        counter = 0;
        double kpos;
        while (inposlog >> kpos)
        {
            kposx[counter] = kpos;
            inposlog >> kpos;
            inposlog >> kpos;
            kposy[counter] = kpos;
            inposlog >> kpos;
            counter++;
        }
        if (counter > 192)
            cout << " --> offset file Wrong! please check..." << endl;
    }
    counter = 0;

    //---------------read combine data---------------------//
    TString comName = fileDir + "/Combine/Dtof-step3-com.root";
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
    fCombine = new TFile(comName, "read");
    fCombineTree = (TTree *)fCombine->Get("ComData");
    fCombineTree->SetBranchAddress("event", &event);
    fCombineTree->SetBranchAddress("trackTOF", &trackTOF);
    fCombineTree->SetBranchAddress("tracktheta", &tracktheta);
    fCombineTree->SetBranchAddress("trackDir", trackDir);
    fCombineTree->SetBranchAddress("T0_Npe", &T0_Npe);
    fCombineTree->SetBranchAddress("T0_TOA", T0_TOA);
    fCombineTree->SetBranchAddress("T0_TOT", T0_TOT);
    fCombineTree->SetBranchAddress("T0_trackPos", T0_trackPos);
    fCombineTree->SetBranchAddress("DTOF_Npe", &DTOF_Npe);
    fCombineTree->SetBranchAddress("DTOF_TOA", DTOF_TOA);
    fCombineTree->SetBranchAddress("DTOF_TOT", DTOF_TOT);
    fCombineTree->SetBranchAddress("DTOF_trackPos", DTOF_trackPos);
    //----------------T0 reconstruction--------------------//
    TString recT0Name = fileDir + "/Combine/Dtof-step4-recT0.root";
    fRecT0 = new TFile(recT0Name, "RECREATE");
    fRecT0Tree = new TTree("T0rec", "T0rec");
    fRecT0Tree->Branch("event", &event, "event/I");
    fRecT0Tree->Branch("T01time", &timet1, "T01time/D");
    fRecT0Tree->Branch("T02time", &timet2, "T02time/D");
    //----------------T0 Cali if needed--------------------//
    TString caliName = fileDir + "/Combine/T0recCali.root";
    fCaliRec = new TFile(caliName, "RECREATE");

    fitgaus = new TF1("fitgaus", "gaus");
    //---------------DTOF data histogram-------------------//
    TString recDTOFName=fileDir+"/Combine/Dtof-step4-recDTOF.root";
    fRecDTOF = new TFile(recDTOFName,"RECREATE");
    histChannelT[0]=new TH2D("channelT-pi","channelT-pi",224,0,224,1000,-4000,8000);
    histChannelT[1]=new TH2D("channelT-p","channelT-p",224,0,224,1000,-4000,8000);
    //---------------Data sample---------------------------//
    TString sampleName=fileDir+"/Combine/SampleFor_Pi_P.root";
    fSample=new TFile(sampleName,"RECREATE");
    fSampleTree[0]=new TTree("sample_pi","sample_pi");
    fSampleTree[1]=new TTree("sample_p","sample_p");
    for(int i=0;i<2;i++)
    {
        fSampleTree[i]->Branch("DTOF_TOA",DTOF_TOA,"DTOF_TOA[224]/D");
        fSampleTree[i]->Branch("DTOF_Npe",&DTOF_Npe,"DTOF_Npe/I");
    }

    return true;
}
void Rectool::Loop()
{

    TH1D *histTime = new TH1D("T2-T1", "", 5000, 10000, 40000);
    TH1D *deltaLR[4];
    TH2D *deltaTX[4];
    TH2D *deltaTY[4];

    TH2D *hDTOFcht[2];
    hDTOFcht[0] = new TH2D("DTOFchannel_t_pi", "DTOFchannel_t_pi", 224, 0, 224, 500, -4000, 1000);
    hDTOFcht[1] = new TH2D("DTOFchannel_t_p", "DTOFchannel_t_p", 224, 0, 224, 500, -4000, 1000);

    for (int i = 0; i < 4; i++)
    {
        deltaLR[i] = new TH1D(Form("L-R_%d", i), Form("L-R_%i", i), 500, -2000, 2000);
        deltaTX[i] = new TH2D(Form("L-R_vs_X_T0%d", i), Form("L-R_vs_X_T0%d", i), 500, -100, 100, 500, -2000, 2000); // mm~ps
        deltaTY[i] = new TH2D(Form("L-R_vs_Y_T0%d", i), Form("L-R_vs_Y_T0%d", i), 500, -100, 100, 500, -2000, 2000); // mm~ps
    }

    cout << " --> Process Combine Result file for T0_rec..." << endl;
    for (int i = 0; i < fCombineTree->GetEntries(); i++)
    {
        Reset(); // make sure the vectors and parameters are ready
        fCombineTree->GetEntry(i);
        DrawProcessbar(i, fCombineTree->GetEntries());
        if (T0_Npe < 160)
        { // 96 is a half
            ResetT0Data();
            fRecT0Tree->Fill(); // empty
            continue;
        }
        for(int i=0;i<224;i++){// clear DTOF_TOA
           if(DTOF_TOA[i]<-2.15*1e6||DTOF_TOA[i]>-1.95*1e6) DTOF_TOA[i]=0;
        }
        if (ReconstructT0())
        { /// fill T0 result
            fRecT0Tree->Fill();
            histTime->Fill(timet2-timet1);
            for (int i = 0; i < 4; i++)
            {
                deltaLR[i]->Fill(TL[i] - TR[i]);
                deltaTX[i]->Fill(T0_trackPos[i / 2 * 2], TL[i] - TR[i]);
                deltaTY[i]->Fill(T0_trackPos[i / 2 * 2 + 1], TL[i] - TR[i]);
            }
        }
        if(DTOF_Npe>=5){
            SubstractT0fromDTOF();
            FillHistDTOF();
            if(timet2-timet1>cutline)
                fSampleTree[0]->Fill();//pi
            else
                fSampleTree[1]->Fill();//p
        }
            

    }
    fRecT0->WriteTObject(histTime);
    fRecT0->WriteTObject(fRecT0Tree);
    for (int i = 0; i < 4; i++)
        fRecT0->WriteTObject(deltaLR[i]);
    for (int i = 0; i < 4; i++)
    {
        deltaTX[i]->Fit(fitline, "Q", "", -500, 500);
        fRecT0->WriteTObject(deltaTX[i]);
    }

    for (int i = 0; i < 4; i++)
    {
        deltaTY[i]->Fit(fitline, "Q", "", -1000, 1000);
        fRecT0->WriteTObject(deltaTY[i]);
    }
    //////////////DTOF likelihood
    histChannelT[0]->Scale(1/histChannelT[0]->Integral());
    histChannelT[1]->Scale(1/histChannelT[1]->Integral());
    for(int i=0;i<2;i++){
        fRecDTOF->WriteTObject(histChannelT[i]);
        fSample->WriteTObject(fSampleTree[i]);
    }    
    return;
}
bool Rectool::ReconstructT0()
{

    // cout<<"event begin"<<endl;
    double temp = 0;
    double xhit = 0;
    double yhit = 0;
    int m = 0;
    for (int j = 0; j < 192; j++)
    {
        // cout<<T0_TOA[j]<<" "<<j<<endl;
        m = j / 48 / 12;
        if ((T0_TOA[j] < 1e-11 && T0_TOA[j] > -1e-11))
            continue;

        if (j < 96)
        {
            xhit = T0_trackPos[0];
            yhit = T0_trackPos[1];
        }
        else
        {
            xhit = T0_trackPos[2];
            yhit = T0_trackPos[1];
        }
        temp = T0_TOA[j] - offset[j]-pow(-1,j/48)*(k[m]*xhit+b[m])/2;//- kposx[j] * xhit - kposy[j] * yhit;//;; //; //-pow(-1,j/48)*(k[m]*xhit+b[m])/2;//+(xhit-b[m])/(2*k[m]);
        switch (j / 24)
        {
        case 0:
            T01_plate1_vec.push_back(temp);
            T0left[0].push_back(temp);
            break;
        case 1:
            T01_plate2_vec.push_back(temp);
            T0left[1].push_back(temp);
            break;
        case 2:
            T01_plate1_vec.push_back(temp);
            T0right[0].push_back(temp);
            break;
        case 3:
            T01_plate2_vec.push_back(temp);
            T0right[1].push_back(temp);
            break;

        case 4:
            T02_plate1_vec.push_back(temp);
            T0left[2].push_back(temp);
            break;
        case 5:
            T02_plate2_vec.push_back(temp);
            T0left[3].push_back(temp);
            break;
        case 6:
            T02_plate1_vec.push_back(temp);
            T0right[2].push_back(temp);
            break;
        case 7:
            T02_plate2_vec.push_back(temp);
            T0right[3].push_back(temp);
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

    time[0] = CalVectorMean(T01_plate1_vec, 16, 0);
    time[1] = CalVectorMean(T01_plate2_vec, 16, 0);
    time[2] = CalVectorMean(T02_plate1_vec, 16, 0);
    time[3] = CalVectorMean(T02_plate2_vec, 16, 0);
    // cout<<timet22-timet11<<endl;
    timet1 = (time[0] + time[1]) / 2;
    timet2 = (time[2] + time[3]) / 2;
  
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
    return true;
}
void Rectool::Reset()
{
    memset(T0_TOA, 0, sizeof(T0_TOA));
    memset(T0_TOT, 0, sizeof(T0_TOT));
    memset(DTOF_TOA, 0, sizeof(DTOF_TOA));
    memset(DTOF_TOT, 0, sizeof(DTOF_TOT));
    T01_plate1_vec.clear();
    T01_plate2_vec.clear();
    T02_plate1_vec.clear();
    T02_plate2_vec.clear();
    timet1 = 0;
    timet2 = 0;
    for (int i = 0; i < 4; i++)
    {
        T0left[i].clear();
        T0right[i].clear();
        TL[i] = 0;
        TR[i] = 0;
    }
    return;
}

void Rectool::ResetT0Data()
{
    timet1 = 0;
    timet2 = 0;
}

void Rectool::Calibration()
{
    ofstream output;
    output.open("calibration.txt");
    TH2D *hXT[192];
    TH2D *hYT[192];
    ofstream outlog;
    outlog.open("T0channelCali.txt"); // from channel 0 to channel 192
    double xhit, yhit;
    for (int i = 0; i < 192; i++)
    {
        hXT[i] = new TH2D(Form("T0recCalix_%d", i), Form("T0recCalix_%d", i), 500, -100, 100, 500, -3000, 3000);
        hYT[i] = new TH2D(Form("T0recCaliy_%d", i), Form("T0recCaliy_%d", i), 500, -100, 100, 500, -3000, 3000);
    }

    TH1D *histT01[96];
    TH1D *histT02[96];
    for (int i = 0; i < 96; i++)
    {
        histT01[i] = new TH1D(Form("channel_%d_T01", i), Form("channel_%d_T01", i), 500, -5000, 5000);
    }
    for (int i = 0; i < 96; i++)
    {
        histT02[i] = new TH1D(Form("channel_%d_T02", i), Form("channel_%d_T02", i), 500, -5000, 5000);
    }

    cout << " --> begin to Calibration T0 for RecFile..." << endl;
    for (int i = 0; i < fCombineTree->GetEntries(); i++)
    {
        DrawProcessbar(i, fCombineTree->GetEntries());
        Reset(); // make sure the vectors and parameters are ready
        fCombineTree->GetEntry(i);
        if (T0_Npe < 160)
        { // 96 is a half
            continue;
        }

        if (ReconstructT0())
        { /// fill T0 result;
            for (int j = 0; j < 192; j++)
            {
                double tt = 0;
                if (j < 96)
                    tt = timet1;
                else
                    tt = timet2;
                hXT[j]->Fill(T0_trackPos[j / 96 * 2], T0_TOA[j] - tt);
                hYT[j]->Fill(T0_trackPos[j / 96 * 2 + 1], T0_TOA[j] - tt);
            }
            for (int j = 0; j < 96; j++)
            {

                switch (j / 24)
                {
                case 0:
                    if (T0_TOA[j] > 1e-11 || T0_TOA[j] < -1e-11)
                        histT01[j]->Fill(T0_TOA[j] - offset[j] - time[1]);
                    // cout<<"deltaT "<<T0_TOA[j]-timet12<<endl;
                    if (T0_TOA[j + 96] > 1e-11 || T0_TOA[j + 96] < -1e-11)
                        histT02[j]->Fill(T0_TOA[j + 96] - offset[j + 96] - time[3]);
                    break;
                case 1:
                    if (T0_TOA[j] > 1e-11 || T0_TOA[j] < -1e-11)
                        histT01[j]->Fill(T0_TOA[j] - offset[j] - time[0]);
                    if (T0_TOA[j + 96] > 1e-11 || T0_TOA[j + 96] < -1e-11)
                        histT02[j]->Fill(T0_TOA[j + 96] - offset[j + 96] - time[2]);
                    break;
                case 2:
                    if (T0_TOA[j] > 1e-11 || T0_TOA[j] < -1e-11)
                        histT01[j]->Fill(T0_TOA[j] - offset[j] - time[1]);
                    if (T0_TOA[j + 96] > 1e-11 || T0_TOA[j + 96] < -1e-11)
                        histT02[j]->Fill(T0_TOA[j + 96] - offset[j + 96] - time[3]);
                    break;
                case 3:
                    if (T0_TOA[j] > 1e-11 || T0_TOA[j] < -1e-11)
                        histT01[j]->Fill(T0_TOA[j] - offset[j] - time[0]);
                    if (T0_TOA[j + 96] > 1e-11 || T0_TOA[j + 96] < -1e-11)
                        histT02[j]->Fill(T0_TOA[j + 96] - offset[j + 96] - time[2]);
                    break;
                default:
                    break;
                }
            }
        }
    }
    for (int i = 0; i < 192; i++)
    {
        hXT[i]->Fit(fitline, "Q");
        output << fitline->GetParameter(0) << " " << fitline->GetParameter(1) << " ";
        fCaliRec->WriteTObject(hXT[i]);
        // kposx[i] = fitline->GetParameter(0);
        hYT[i]->Fit(fitline, "Q");
        output << fitline->GetParameter(0) << " " << fitline->GetParameter(1) << endl;
        fCaliRec->WriteTObject(hYT[i]);
        // kposy[i] = fitline->GetParameter(0);
    }

    for (int i = 0; i < 96; i++)
    {
        histT01[i]->Fit(fitgaus, "Q");
        fCaliRec->WriteTObject(histT01[i]);
        outlog << fitgaus->GetParameter(1) << endl;
    }
    // outlog << " " << endl;
    for (int i = 0; i < 96; i++)
    {
        histT02[i]->Fit(fitgaus, "Q");
        fCaliRec->WriteTObject(histT02[i]);
        outlog << fitgaus->GetParameter(1) << endl;
    }

    cout << " --> CaliRec Success!" << endl;

    return;
}
void Rectool::CalOffsetPos()
{
    return;
}
void Rectool::CaliT0Pos(int i, double hitx, double hity)
{
    T0_TOA[i] -= kposx[i] * hitx + kposy[i] * hity;
    return;
}
void Rectool::FillHistDTOF()
{
    for(int i=0;i<224;i++){
        if(DTOF_TOA[i]==0||DTOF_TOA[i]<-1e4||DTOF_TOA[i]>5e4)
            continue;
        if(timet2-timet1>cutline)
            histChannelT[0]->Fill(i,DTOF_TOA[i]);
        else
            histChannelT[1]->Fill(i,DTOF_TOA[i]);
    }
    return;
}
void Rectool::SubstractT0fromDTOF(){
    for(int i=0;i<224;i++){
        if(DTOF_TOA[i]!=0)
            DTOF_TOA[i]-=(timet1+timet2)/2.;
    }
}