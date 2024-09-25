#ifndef CRINFO_h
#define CRINFO_h 1

#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>

#include "TSystem.h"
#include "TApplication.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TGaxis.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TCanvas.h"
#include "TVector3.h"
#include "TBenchmark.h"
#include "TLatex.h"
#include "TLegend.h"
#include "TRandom3.h"

#include "CRConst.h"

using namespace std;

const int eventrange = 0x10000; /// 65536
const TString asic2detName = "pars/asic2det_64to384.csv";
const TString MMalignmentPar3Name = "pars/MMalignmentPar3.dat";
const TString MMalignmentPar6Name = "pars/MMalignmentPar6.dat";

const double BETA = 3000. / TMath::Sqrt(3000. * 3000. + Mass_mu * Mass_mu); // p = 3 GeV/c
const char detName[6][10] = {"MM0", "MM1", "MM2", "MM3", "T0", "DTOF"};
const double Xoff[6] = {0, 0, 0, 0, 0, 0};
const double Yoff[6] = {0, 0, 0, 0, 0, 0};
// const double Yoff[NDET + 2] = {90, 90, -35, -35, 45, 0}; // 20 degree angle
//  old position
// const double Zpos[6] = {582, 538, -120, -164, 320, 26}; // MM-0-3, T0, DTOF

// position for beam test setup==0
const double Zpos[7] = {2207, 2067, -3947, -4147, 3357, -4922, 0}; // MM-0-3, T01,T02, DTOF
// position for beam test setup==1
//  const double Zpos[7] = {0, -200, -870, -1070, 3357, -4922, 0}; // MM-0-3, T01,T02, DTOF

const double rmscut = 5;    // for MM waveform cut
const int max_Nch_use = 10; // for MM dec

const int color[21] = {1, 2, 3, 4, 6, 7, 9, 30, 50, 51, 28, 38, 41, 39, 42, 20, 40, 5, 8, 29, 33};

double Np(double WaveLength); // unit of WaveLength: nm
double Ng(double WaveLength); // unit of WaveLength: nm

TString GetPath(TString fileName);
TString GetFilename(TString fileName);
void GetFileList(TString filePath, TString filePattern, vector<TString> &fList);
ofstream LogFile(TString fileName);
void DrawProcessbar(int i, int total);
int IsMatched(int trigid, vector<int> idvec);
int IsMatched(double RecT, vector<double> RecTs);
TString XYname(int findex);
TLatex *CreatLatex(TString text, double x = 0.65, double y = 0.5, int textFont = 42);

Double_t CalVectorMean(vector<double> Time, int nn = 0, int bb = 0);
void SortRecT(vector<double> &TOAevent);
double SelectRecT(vector<double> &RecT, double deltaT = 1500, double w = 1);
#endif
