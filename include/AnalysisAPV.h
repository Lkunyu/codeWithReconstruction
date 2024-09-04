#ifndef AnalysisAPV_h
#define AnalysisAPV_h 1

#include "CRInfo.h"

class AnalysisAPV
{
  ///// process wave to get ped, pedrms, maxA
  public:
    TFile *frawFile;
    TTree * frawTree;
    TFile *fanaFile;
    TTree * fanaTree[8];
    AnalysisAPV(TString fRawName, TString fAnaName, int force=0);
    ~AnalysisAPV();
    bool Cut();
    bool Init();
    void Loop();
    int IndexXY(int board, int chip);
    int IndexDet(int board, int chip);
    int IndexStrip(int board,int chip,int channel);
    int CalculateLostID(vector<int> eventvec);
    void InitData();
    void riseTandTOT(vector<double> waveform);
    void CalWaveInfo();
  private:
    TString rawName;
    TString anaName;
    int force;
    /// input data
    int event;
    int board;
    int chip;
    int channel;
    vector<short> *wave=0;
    TBranch *b_event;
    TBranch *b_board;
    TBranch *b_chip;
    TBranch *b_channel;
    TBranch *b_wave;
    /// output data
    double maxA;
    int maxT;
    double ped;
    double pedrms;
    double riseT;
    double TOT;
    int XY;
    int Det; 
    vector<int> APVmap;

};

#endif
