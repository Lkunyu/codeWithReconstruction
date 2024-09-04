#include "DtofDat2Root.h"

using namespace std;

DtofDat2Root::DtofDat2Root(vector<TString> fList, TString fRawName, int f_force)
{
  datList = fList;
  rawName = fRawName;
  force = f_force;
}

DtofDat2Root::~DtofDat2Root()
{
}

bool DtofDat2Root::Init()
{
  if (force != 1)
  {
    FileStat_t fStat;
    gSystem->GetPathInfo(rawName, fStat);
    if (fStat.fSize != 0)
    {
      cout << " ERROR! " << rawName << " is exist!" << endl;
      return false;
    }
  }
  frawFile = new TFile(rawName, "recreate"); // create output file
  if (!frawFile->IsOpen())
  {
    cout << " ERROR! " << rawName << " cant open!" << endl;
    return false;
  }

  frawTree = new TTree("DtofRaw", "DtofRaw"); // create tree
  frawTree->Branch("event", &event);
  frawTree->Branch("board", &board);
  frawTree->Branch("channel", &channel);
  frawTree->Branch("TOA", &TOA);
  frawTree->Branch("TOT", &TOT);
  frawTree->Branch("tempFPGA", &tempFPGA);
  frawTree->Branch("tempSFP", &tempSFP);

  return true;
}

void DtofDat2Root::Loop()
{
  ofstream op = LogFile(rawName);
  int length = sizeof(unsigned int);
  unsigned int memblock = 0xffff;
  int memblocks[9];

  long preevent[2] = {0};
  int carrynum[2] = {0};
  int totallines = 0;

  for (int i = 0; i < (int)datList.size(); i++)
  {
    fstream InDat;
    InDat.open(datList[i], ios::in | ios::binary);
    if (!InDat.is_open())
    {
      cout << " ERROR! " << datList[i] << " cant open!" << endl;
      continue;
    }
    cout << " --> Reading a new dat file-" << i << " : " << datList[i] << endl;
    op << " --> Reading a new dat file-" << i << " : " << datList[i] << endl;

    // read row and column
    for (int jj = 0; jj < 2; jj++)
    {
      InDat.read((char *)(&memblock), length); /// read 32 bits
      if (jj == 0)
        totallines = memblock;
    }

    vector<int> events[2], boards[2], channels[2];
    vector<double> TOAs[2], TOTs[2], tempFPGAs[2], tempSFPs[2];
    int start = 65536*200, end = 0;
    // read data
    for (int jj = 0; jj < totallines; jj++)
    {
      for (int kk = 0; kk < 9; kk++)
      {
        InDat.read((char *)(&memblock), length); /// read 32 bits
        memblocks[kk] = memblock;
      }

      InitData();
      board = memblocks[0];
      channel = memblocks[1];

      // board 0-11 group 1, board 12-23 group 2
      int group = 0;
      if (board >= 12)
        group = 1;
      // board 8 and 20 lost
      if (board > 8 && board < 20)
        board -= 1;
      if (board > 20)
        board -= 2;

      if (board == 0 && channel == 101)
      {
        tempFPGA = memblocks[2] / 65536.;
        tempSFP = memblocks[3] / 65536.;
        cout << " --> Master board temperature, board 1 =  " << tempFPGA << ", board 2 =  " << tempSFP << endl;
        op << " --> Master board temperature, board 1 =  " << tempFPGA << ", board 2 =  " << tempSFP << endl;
        frawTree->Fill();
      }
      else
      {
        if (board == 0 && channel == 100)
        {
          event = memblocks[2];
          // TOA = memblocks[4] * 1562.5;
          TOA = (memblocks[4] ) * 1562.5; //memblocks[3] * pow(2, 30)
          // cout<<"event "<<memblocks[2]<<" TOA course "<<memblocks[3]<<" fine "<<memblocks[4]<<" "<<memblocks[5]<<" "<<memblocks[6]<<endl;
        }
        else
        {
          TOA = (memblocks[2] - memblocks[3] / 262143.) * 1562.5; // 640Mhz, 1562.5ps
          TOT = (memblocks[4] - memblocks[5] / 262143.) * 1562.5 - TOA;
          // cout<<"T0 TOA course "<<memblocks[2]<<" fine "<<memblocks[3]<<endl;
          // if(board<16)
          //  TOA += memblocks[7] * pow(2, 30) * 1562.5;
          tempFPGA = memblocks[6] * 509.3140064 / 65536. - 280.2308787;
          tempSFP = memblocks[7] / 256.;
          event = memblocks[8];
        }

        if (IsCarry(event, preevent[group]))
          carrynum[group]++;
        preevent[group] = event;
        event = carrynum[group] * eventrange + event; // eventrange = 65536

        if (event > end)
          end = event;
        if (event < start)
          start = event;
        events[group].push_back(event);
        boards[group].push_back(board);
        channels[group].push_back(channel);
        TOAs[group].push_back(TOA);
        TOTs[group].push_back(TOT);
        tempFPGAs[group].push_back(tempFPGA);
        tempSFPs[group].push_back(tempSFP);
        // frawTree->Fill();
      }
    } /// !InDat.eof()

    int pos[2] = {0};
    for (int jj = start; jj <= end; jj++)
    {
      for (int group = 0; group < 2; group++)
      {
        for (int kk = pos[group]; kk < events[group].size(); kk++)
        {
          if (events[group][kk] > jj)
          {
            pos[group] = kk;
            break;
          }
          if (events[group][kk] == jj)
          {
            event = events[group][kk];
            board = boards[group][kk];
            channel = channels[group][kk];
            TOA = TOAs[group][kk];
            TOT = TOTs[group][kk];
            tempFPGA = tempFPGAs[group][kk];
            tempSFP = tempSFPs[group][kk];
            frawTree->Fill();
          }
        } // signal loop
      } // group loop
    } // event loop

  } /// dataList loop

  cout << " --> Total signal = " << frawTree->GetEntries() << endl;
  op << " --> Total signal = " << frawTree->GetEntries() << endl;

  frawFile->cd();
  frawTree->Write();
  frawFile->Flush();
  frawFile->Close();
  cout << " --> DTOF Raw data file has been convert to root-file: "
       << rawName << endl;
  op << " --> DTOF Raw data file has been convert to root-file: "
     << rawName << endl;
  op.close();
}

void DtofDat2Root::InitData()
{
  event = 0;
  board = 0;
  channel = 0;
  TOA = 0;
  TOT = 0;
  tempFPGA = 0;
  tempSFP = 0;
}

unsigned short DtofDat2Root::DatExchange(unsigned short memblock)
{
  unsigned short tmp = memblock;
  unsigned short low = memblock & 0xff;
  unsigned short high = (memblock >> 8) & 0xFF;
  tmp = low;
  tmp <<= 8;
  tmp |= high;
  return tmp;
}
bool DtofDat2Root::IsCarry(int event, int preevent)
{
  if (event - preevent < -2e4 && event >= 0 && event < 50 && preevent > 65530)
  {
    return true;
  }
  else
    return false;
}
