#ifndef T0tool_h
#define T0tool_h 1

#include "CRInfo.h"

using namespace std;

class T0tool // process T0data using position can calculate T0offset
{
public:
    T0tool();
    T0tool(TString fileDir, int force);
    ~T0tool();
    bool Init();
    void Loop();
    void SetFileDir(TString fileDirectory) { fileDir = fileDirectory; }
    void Calibration();
    void ResetData();
private:
    bool TrackerFlag = 0; // 0 means do not use
    double offset[192];
    TString fileDir;
    TFile *T0dataIn;
    TTree *T0dataTree;
    Int_t event;
    Int_t Npe;
    Double_t T0TOA[192];
    Double_t T0TOT[192];
    Int_t particletype; // 0 means nothing, 1 means mu , 2 means p ,3 means pi
    Double_t time0, time1;
    
    TFile *T0dataOut;
    TTree *T0dataOutTree;
    TF1 *fitgaus;
    

};
#endif