#include "TFile.h"
#include "TH2D.h"
#include "TTree.h"
#include <iostream>
#include "unistd.h"
#include "TMath.h"
#include <stdio.h>
#include <numeric>
#include "TRandom3.h"
int openTBrowser(){
    TBrowser* B=new TBrowser();
    B->Draw();
    return 0;
}