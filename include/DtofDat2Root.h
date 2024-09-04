#ifndef DTOFDAT2ROOT_h
#define DTOFDAT2ROOT_h 1

#include "CRInfo.h"

using namespace std;

class DtofDat2Root
{
  public:
    TFile *frawFile;
    TTree *frawTree;
    
    DtofDat2Root(vector<TString> fList, TString fRawName, int force=0);
    ~DtofDat2Root();
    bool Init();
    void InitData();
    void Loop();
    unsigned short DatExchange(unsigned short memblock);
    bool IsCarry(int event, int preevent);

  private:
    TString rawName;
    vector<TString> datList;
    int force;

    // output date
    int event;
    int board;
    int channel;
    double TOA;
    double TOT;
    double tempFPGA;
    double tempSFP;

};

#endif
