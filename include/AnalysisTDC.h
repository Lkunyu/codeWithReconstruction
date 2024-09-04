#ifndef ANALYSISTDC_h
#define ANALYSISTDC_h 1

#include "CRInfo.h"

class AnalysisTDC
{
  ///// process wave to get ped, pedrms, maxA
  public:
    TFile *frawFile;
    TTree * frawTree;
    TFile *fanaFile;
    TTree * fanaTree;
    AnalysisTDC(TString fRawName, TString fAnaName, int force=0);
    ~AnalysisTDC();
    bool Cut();
    bool Init(TString caliparsName);
    void Loop();
    void InitData();
    bool getCaliPars(TString filename);
    void calibration();
    void calibrationDTOF();
    void TOTcalibration();
    void tempcalibration();
    void mastertempcalibration();

  private:
    TString rawName;
    TString anaName;
    int force;

    double triggertime = 0;
    double triggertimelow = 0;
    double tempMaster1 = 0;
    double tempMaster2 = 0;
    /// input data
    int event;
    int board;
    int channel;
    double TOA;
    double TOT;
    double tempFPGA;
    double tempSFP;
    TBranch *b_event;
    TBranch *b_board;
    TBranch *b_channel;
    TBranch *b_TOA;
    TBranch *b_TOT;
    TBranch *b_tempFPGA;
    TBranch *b_tempSFP;
    /// output data
    int T0_Npe;
    double T0_TOT[4];
    double T0_TOA[4];
    double T0_temp[4];
    int DTOF_Npe;
    double DTOF_TOT[672];
    double DTOF_TOA[672];
    double DTOF_temp[672];
    /// calibration pars
    double master_temp_par[21];
    double T0_TOT_max[5];
    double T0_TOT_mean[5];
    double T0_TOT_par[5];
    double T0_temp_par[5];
    double T0_offset[5];
    double DTOF_TOT_max[672];
    double DTOF_TOT_mean[672];
    double DTOF_TOT_par[672];
    double DTOF_temp_par[672];
    double DTOF_offset[672];

};

#endif
