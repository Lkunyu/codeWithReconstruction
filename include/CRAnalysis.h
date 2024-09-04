#ifndef CRANALYSIS_h
#define CRANALYSIS_h 1

#include "CRInfo.h"
#include "Separationtool.h"

void ReadTracker(TString fileDir, int force = 0);
void ReadDTOF(TString fileDir, int force = 0);
void ReadData(TString fileDir,int force=0);
void T0result(TString fileDir,int force=0);
void T0CaliBration(TString fileDir,int force=0);
void CombineData(TString fileDir, int force = 0);
//void RecTDC(TString fileDir, int force = 0);
//void RecSim(TString fileDir, int force = 0);
void RecData(TString fileDir, int force=0);
//void UnzipAGET(TString fileDir, int force = 0);
//void UnzipTDC(TString fileDir, int force = 0);
#endif
