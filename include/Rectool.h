#ifndef T0Rectool_h
#define T0Rectool_h 1
#include "CRAnalysis.h"
#include "CRInfo.h"
class Rectool
{
public:
    Rectool(TString fileDirectory,int f_force);
    ~Rectool();
    bool Init();
    void Loop();
    bool ReconstructT0();
    void Reset();
    void ResetT0Data();
    void Calibration();
    void CalOffsetPos();
    void CaliT0Pos(int i,double xhit,double yhit);//directly substract offset from TOA
    void FillHistDTOF();
    void SubstractT0fromDTOF();
private:
    TFile* fCombine;
    TTree* fCombineTree;
    int event;
    double tracktheta;
    double trackTOF;
    double trackDir[3];
    int T0_Npe;
    double T0_TOA[192];
    double T0_TOT[192];
    double T0_trackPos[4];//0,1 for first
    int DTOF_Npe;
    double DTOF_TOA[224];
    double DTOF_TOT[224];
    double DTOF_trackPos[2];
    TFile* fRecT0;
    TTree* fRecT0Tree;
    double timet1,timet2;
    TString fileDir;
    int force;
    TFile* fRecDTOF;
    TTree* fRecDTOFTree;
    double offset[192];
    double offsetPos[192];
   vector<double> T01_plate1_vec, T01_plate2_vec, T02_plate1_vec, T02_plate2_vec;
    vector<double> T0left[4], T0right[4];
    double time[4];//1 T01_1;2 T01_2;3 T02_1;4 T02_2
    double TL[4], TR[4];
    double k[4],b[4];
    TF1* fitline;
    TFile* fCaliRec;
    double kposx[192],kposy[192];
    TF1 *fitgaus;
    ///DTOF hist
    TH2D* histChannelT[2];// 0 for pi,1 for proton
    double cutline=26600;//T2-T1 ps ,over is proton, below is pion
    //2 Data output file pion and p
    TFile* fSample;
    TTree* fSampleTree[2];//0 for pion, and 1 for p
    

};
#endif