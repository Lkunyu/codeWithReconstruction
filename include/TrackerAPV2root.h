#ifndef TrackerAPV2root_h
#define TrackerAPV2root_h 1
using namespace std;
class TrackerAPV2root
{
public: 
    TrackerAPV2root();
    TrackerAPV2root(vector<TString> fList, TString fRawName, int force=0);
    ~TrackerAPV2root();
    bool Init();
    void BlankData();
    void Loop();//to raw data
private:
    vector<TString> datList;
    TString rawName;
    TFile* TrackerRawFile;
    TTree* TrackerRawTree;
    Int_t eventall;
    Int_t board=0;
    Int_t chip;
    Int_t channel;
    vector<short> wave;
    unsigned short exchangehighlow(unsigned short data);
    struct datastruct
    {
        int eventevent;
        int boardboard;
        int chipchip;
        int channelchannel;
        vector<short> wavewave;

    };
    void FillStruct(struct datastruct &datatemp)
    {
        datatemp.wavewave.clear();
        datatemp.eventevent=eventall;
        datatemp.boardboard=board;
        datatemp.chipchip=chip;
        datatemp.channelchannel=channel;
        datatemp.wavewave=wave;
    }
    void StructOut(struct datastruct datatemp)
    {
        wave.clear();
        board=datatemp.boardboard;
        chip=datatemp.chipchip;
        channel=datatemp.channelchannel;
        wave=datatemp.wavewave;
    }
    
};
#endif