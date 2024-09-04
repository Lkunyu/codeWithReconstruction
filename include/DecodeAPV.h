#ifndef DECODEAPV_h
#define DECODEAPV_h 1

#include "CRInfo.h"
#include "APV2DetStrip.h"

class DecodeAPV
{
  public:
    TFile *fanaFile;
    TTree * fanaTree[8];
    TFile *fdecFile;
    TTree * fdecTree[4];
    DecodeAPV(TString fanaName, TString fdecName, int force);
    ~DecodeAPV();
    bool Cut();
    bool Init();
    void Loop();
    void InitData();
    APV2DetStrip *dechit;
    bool CalHit(vector<int> ch,vector<double> amp,int XY);

  private:
    TString anaName;
    TString decName;
    int force;
    /// input data
    int event;
    double maxA;
    int maxT;
    double ped;
    double pedrms;
    double riseT;
    double TOT;
    int channel;
    TBranch *b_event;
    TBranch *b_maxA;
    TBranch *b_maxT;
    TBranch *b_ped;
    TBranch *b_pedrms;
    TBranch *b_channel;
    TBranch *b_riseT;
    TBranch *b_TOT;
    /// ouput data
    bool sig;
    bool sig_x;
    bool sig_y;
    double x;
    double y;
    double z;
    double hit_amp_x;
    double hit_amp_y;
    int hit_strip_num_x;
    int hit_strip_num_y;
    int x_nhits;
    int y_nhits;
    vector<double> x_other;
    vector<double> y_other;
};

#endif 
