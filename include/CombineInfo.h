#ifndef CombineInfo_h
#define CombineInfo_h 1

#include "CRInfo.h"

class CombineInfo
{
  ///// process wave to get ped, pedrms, maxA
  public:
    TFile *ftrackFile;
    TTree *ftrackTree;
    TFile *fT0File;
    TTree *fT0Tree;
    TFile *fDTOFFile;
    TTree *fDTOFTree;
    TFile *fcomFile;
    TTree *fcomTree;
    CombineInfo(TString fTrackName,TString fT0Name, TString fDTOFName, TString fComName, int force=0);
    ~CombineInfo();
    bool Init();
    void Loop();
    void InitData();
    void RecTrackHit();

  private:
    TString trackName;
    TString T0Name;
    TString DTOFName;
    TString comName;
    int force;

    /// input data
    int event;
    int T0_Npe;
    double T0_TOT[192];
    double T0_TOA[192];
    int DTOF_Npe;
    double DTOF_TOT[224];
    double DTOF_TOA[224];
    double DTOF_temp[244];

    double MMx[4];
    double MMy[4];
    double MMz[4];
    bool MMsigx[4];
    bool MMsigy[4];
    double XfitPars[3];
    double YfitPars[3];
    bool useflag=0;

    TBranch *b_event_tdc;
    TBranch *b_T0_Npe;
    TBranch *b_T0_TOT;
    TBranch *b_T0_TOA;
    TBranch *b_T0_temp;
    TBranch *b_DTOF_Npe;
    TBranch *b_DTOF_TOT;
    TBranch *b_DTOF_TOA;
    TBranch *b_DTOF_temp;
    TBranch *b_event_track;
    TBranch *b_MMx;
    TBranch *b_MMy;
    TBranch *b_MMz;
    TBranch *b_MMsigx;
    TBranch *b_MMsigy;
    TBranch *b_XfitPars;
    TBranch *b_YfitPars;

    /// output data
    double DTOF_trackPos[2];
    double T0_trackPos[4];//0,1 for T01 x,y;2,3 for T02 x,y
    double trackDir[3];
    double tracktheta;
    double trackTOF;

};

#endif
