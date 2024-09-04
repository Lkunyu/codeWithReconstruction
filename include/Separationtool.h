#ifndef T0data_h
#define T0data_h 1

#include "CRInfo.h"

using namespace std;

class Separationtool//separate T0 and DTOF
{
public:
    Separationtool();
    Separationtool(TString fileDirectory, int force = 0);
    ~Separationtool();
    void SetFileDir(TString fileDirectory){fileDir=fileDirectory;};
    bool Init();
    void ConstructMap();
    void Loop();
private:
    TString MapName;//change channels to T0 or DTOF, a filename, and the file format should be xxx(rawdata) yy(T0 or DTOF file)
    TFile* infile;
    TTree* intree;
    Double_t DTOF_TOA[672];
    Double_t DTOF_TOT[672];
    Int_t eventID;

    TFile* T0Outfile;
    TTree* T0Tree;
    Double_t T0TOA[192];
    Double_t T0TOT[192];
    int NpeT0;

    TFile* DTOFOutfile;
    TTree* DTOFTree;
    Double_t DTOFTOA[224];
    Double_t DTOFTOT[224];
    int NpeDTOF;
    TString fileDir;
    vector<int> channelMapT0;//T0 1~192; DTOF 300~324; others -1;
    vector<int> channelMapDTOF;


};
#endif