#include "CRAnalysis.h"

#include "DtofDat2Root.h"
#include "AnalysisTDC.h"
#include "T0tool.h"
#include "DecodeAPV.h"
#include "ReconstructTrack.h"
#include "AnalysisAPV.h"
#include "TrackerAPV2root.h"
#include "CombineInfo.h"
#include "Rectool.h"

void ReadTracker(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  //  * Read Tracker APV Data *
  TString TrackerRawName;
  TString TrackerAnaName;
  TString TrackerDecName;
  TString TrackerRecName;
  vector<TString> datList;
  if (fileDir.EndsWith(".dat"))
  {
    cout << " --> The Input fileName is:" << fileDir << endl;
    TString PreName;
    PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
    datList.push_back(fileDir);
    cout << " --> The prename is:" << PreName << endl;
    TrackerRawName = PreName + "-Tracker-step1-raw.root";
    TrackerAnaName = PreName + "-Tracker-step2-ana.root";
    TrackerDecName = PreName + "-Tracker-step3-dec.root";
    TrackerRecName = PreName + "-Tracker-step4-rec.root";
  }
  else
  {
    if (fileDir.EndsWith("/"))
      fileDir.Remove(fileDir.Length() - 1, 1);
    cout << " --> The Input fileDir is:" << fileDir << endl;

    TString filePath = fileDir + "/Tracker/";
    GetFileList(filePath, ".dat", datList);

    TrackerRawName = fileDir + "/Combine/Tracker-step1-raw.root";
    TrackerAnaName = fileDir + "/Combine/Tracker-step2-ana.root";
    TrackerDecName = fileDir + "/Combine/Tracker-step3-dec.root";
    TrackerRecName = fileDir + "/Combine/Tracker-step4-rec.root";
  }

  // // #produce raw-root
  // cout << endl
  //      << " --> Convert raw data to root, Output : " << TrackerRawName << endl;
  // TrackerAPV2root fTrackerAPV2root(datList, TrackerRawName, force);
  // if (fTrackerAPV2root.Init())
  //   fTrackerAPV2root.Loop();
  // cout << endl;

  // // #calculate maxA,pedmean,pedrms
  // cout << endl
  //      << " --> Analyzing waveform, Output : " << TrackerAnaName << endl;
  // AnalysisAPV fAnalysisAPV(TrackerRawName, TrackerAnaName, force);
  // if (fAnalysisAPV.Init())
  //   fAnalysisAPV.Loop();
  // cout << endl;

  // // #decode ASIC channel to detector channel
  // cout << endl
  //      << " --> Decoding hits, Output : " << TrackerDecName << endl;
  // DecodeAPV fDecodeAPV(TrackerAnaName, TrackerDecName, force);
  // if (fDecodeAPV.Init())
  //   fDecodeAPV.Loop();
  // cout << endl;

  // #reconstruct track, including track alignment
  cout << endl
       << " --> Reconstruct track, Output : " << TrackerRecName << endl;
  ReconstructTrack fReconstructTrack(TrackerDecName, TrackerRecName, 1);
  if (fReconstructTrack.Init())
    fReconstructTrack.Loop();
  cout << endl;

  cout << " --> Read tracker data successfully !" << endl;

  bench.Show("full");
}

void ReadDTOF(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  //  * Read DTOF Data *
  TString DtofRawName;
  TString DtofAnaName;
  vector<TString> datList;
  if (fileDir.EndsWith(".dat"))
  {
    cout << " --> The Input fileName is:" << fileDir << endl;
    TString PreName;
    PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
    datList.push_back(fileDir);
    cout << " --> The prename is:" << PreName << endl;
    DtofRawName = PreName + "-Dtof-step1-raw.root";
    DtofAnaName = PreName + "-Dtof-step2-ana.root";
  }
  else
  {
    if (fileDir.EndsWith("/"))
      fileDir.Remove(fileDir.Length() - 1, 1);
    cout << " --> The Input fileDir is:" << fileDir << endl;

    TString filePath = fileDir + "/DTOF/";
    GetFileList(filePath, ".dat", datList);

    DtofRawName = fileDir + "/Combine/Dtof-step1-raw.root";
    DtofAnaName = fileDir + "/Combine/Dtof-step2-ana.root";
  }

  // #produce raw-root
  cout << endl
       << " --> Convert raw data to root, Output : " << DtofRawName << endl;
  DtofDat2Root fDtofDat2Root(datList, DtofRawName, force);
  if (fDtofDat2Root.Init())
    fDtofDat2Root.Loop();

  // #analysis TDC data, TOT and temp calibration
  cout << endl
       << " --> Analyzing TDC data to root, Output : " << DtofAnaName << endl;
  AnalysisTDC fAnalysisTDC(DtofRawName, DtofAnaName, force);
  if (fAnalysisTDC.Init("pars/dtof_cali_parsnew2.csv"))
    fAnalysisTDC.Loop();
  cout << endl;

  cout << " --> Read dtof data successfully !" << endl;

  bench.Show("full");
}
///////////put in read data
void ReadData(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  TString DtofAnaName;
  TString DtofOutName;
  TString T0OutName;
  if (fileDir.EndsWith("/"))
    fileDir.Remove(fileDir.Length() - 1, 1);
  cout << " --> The Input fileDir is:" << fileDir << endl;

  DtofAnaName = fileDir + "/Combine/Dtof-step2-ana.root";
  DtofOutName = fileDir + "/Combine/DTOF_Data.root";
  T0OutName = fileDir + "/Combine/T0_Data.root";

  // #produce raw-root
  cout << endl
       << " --> Convert TDC data to root, Output : T0 " << T0OutName << " and DTOF " << DtofOutName << endl;
  Separationtool fSeparation(fileDir, force);
  if (fSeparation.Init())
    fSeparation.Loop();
  cout << endl;

  cout << " --> Combine track and TDC data successfully !" << endl;

  bench.Show("full");
}

void T0result(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  TString T0InName = "/Combine/T0_Data.root";
  TString T0OutName = "/Combine/T0_Result_data.root";
  if (fileDir.EndsWith("/"))
    fileDir.Remove(fileDir.Length() - 1, 1);
  cout << " --> The Input fileName is:" << fileDir << T0InName << endl;

  cout << endl
       << " -->Process T0 data to root, Output : " << T0OutName << endl;
  T0tool fT0tool(fileDir, force);
  if (fT0tool.Init())
    fT0tool.Loop();
  cout << endl;

  cout << " --> T0 process successfully !" << endl;

  bench.Show("full");
}

void T0CaliBration(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  TString T0InName = "/Combine/T0_Data.root";
  TString T0OutName = "/Combine/T0_Cali.root";
  if (fileDir.EndsWith("/"))
    fileDir.Remove(fileDir.Length() - 1, 1);
  cout << " --> The Input fileName is:" << fileDir << T0InName << endl;

  cout << endl
       << " -->Cali T0 data to root, Output : " << T0OutName << endl;
  T0tool fT0tool(fileDir, force);
  if (fT0tool.Init())
    fT0tool.Calibration();
  cout << endl;

  cout << " --> T0 Cali successfully !" << endl;

  bench.Show("full");
}

/////////end put

void CombineData(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  //  * Combine DTOF TDC and Track Data *
  TString TrackerRecName;
  TString DTOFName;
  TString T0Name;
  TString DtofComName;

  if (fileDir.EndsWith("/"))
    fileDir.Remove(fileDir.Length() - 1, 1);
  cout << " --> The Input fileDir is:" << fileDir << endl;

  TrackerRecName = fileDir + "/Combine/Tracker-step4-rec.root";
  DTOFName = fileDir + "/Combine/DTOF_data.root";
  T0Name = fileDir + "/Combine/T0_data.root";
  DtofComName = fileDir + "/Combine/Dtof-step3-com.root";

  // #produce raw-root
  cout << endl
       << " --> Combine track and TDC data to root, Output : " << DtofComName << endl;
  CombineInfo fCombineInfo(TrackerRecName, T0Name, DTOFName, DtofComName, force);
  if (fCombineInfo.Init())
    fCombineInfo.Loop();
  cout << endl;

  cout << " --> Combine track and TDC data successfully !" << endl;

  bench.Show("full");
}

void RecData(TString fileDir, int force)
{
  TBenchmark bench;
  bench.Start("full");
  TString comName = "/Combine/Dtof-step3-com.root";
  TString recName = "/Combine/Dtof-step4-rec.root";
  if (fileDir.EndsWith("/"))
    fileDir.Remove(fileDir.Length() - 1, 1);
  cout << " --> The Input Combine fileName is:" << fileDir << comName << endl;

  cout << endl
       << " --> reconstruction Combine file to root, Output : " << recName << endl;
  Rectool fRectool(fileDir, force);
  if (fRectool.Init()) // fRectool.Calibration();
    fRectool.Loop();
  cout << endl;

  cout << " --> Reconsturction successfully !" << endl;

  bench.Show("full");
}
// void RecTDC(TString fileDir, int force)
// {
//   TBenchmark bench;
//   bench.Start("full");
//   //  * Read DTOF Data *
//   TString DtofComName;
//   TString DtofRecName;
//   if (fileDir.EndsWith(".root"))
//   {
//     cout << " --> The Input fileName is:" << fileDir << endl;
//     TString PreName;
//     PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
//     cout << " --> The prename is:" << PreName << endl;
//     DtofComName = fileDir;
//     DtofRecName = PreName + "-rec.root";
//   }
//   else
//   {
//     if (fileDir.EndsWith("/"))
//       fileDir.Remove(fileDir.Length() - 1, 1);
//     cout << " --> The Input fileDir is:" << fileDir << endl;

//     DtofComName = fileDir + "/Combine/Dtof-step3-com.root";
//     DtofRecName = fileDir + "/Combine/Dtof-step4-rec.root";
//   }

//   // #Reconstruct TDC
//   cout <<endl<<" --> Reconstructing TDC data, Output : " << DtofRecName << endl;
//   ReconstructTDC fReconstructTDC(DtofComName,DtofRecName, force);
//   if(fReconstructTDC.Init()) fReconstructTDC.Loop();
//   cout<<endl;

//   cout<<" --> Reconstructed TDC data successfully !"<<endl;

//   bench.Show("full");
// }

// void RecSim(TString fileDir, int force)
// {
//   TBenchmark bench;
//   bench.Start("full");
//   //  * Read DTOF Data *
//   TString DtofSimName;
//   TString DtofComName;
//   TString DtofRecName;
//   if (fileDir.EndsWith(".root"))
//   {
//     cout << " --> The Input fileName is:" << fileDir << endl;
//     TString PreName;
//     PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
//     cout << " --> The prename is:" << PreName << endl;
//     DtofSimName = fileDir;
//     DtofComName = PreName + "-com.root";
//     DtofRecName = PreName + "-rec.root";
//   }
//   else
//   {
//     if (fileDir.EndsWith("/"))
//       fileDir.Remove(fileDir.Length() - 1, 1);
//     cout << " --> The Input fileDir is:" << fileDir << endl;

//     DtofSimName = fileDir + "/Combine/Dtof-step0-sim.root";
//     DtofComName = fileDir + "/Combine/Dtof-step3-com.root";
//     DtofRecName = fileDir + "/Combine/Dtof-step4-rec.root";
//   }

//   // #Convert Simmulation Data to Exp-type Data
//   cout <<endl<<" --> Converting Sim-Data to Exp-type Data, Output : " << DtofComName << endl;

//   SimData2Rec fSimData2Rec(DtofSimName,DtofComName, force);
//   if(fSimData2Rec.Init()) fSimData2Rec.Loop();
//   cout<<endl;

//   // #Reconstruct TDC
//   cout <<endl<<" --> Reconstructing Sim-Data, Output : " << DtofRecName << endl;
//   ReconstructSimData fReconstructSimData(DtofComName,DtofRecName, force);
//   if(fReconstructSimData.Init()) fReconstructSimData.Loop();
//   cout<<endl;

//   cout<<" --> Reconstructed TDC data successfully !"<<endl;

//   bench.Show("full");
// }

// void UnzipAPV(TString fileDir, int force)
// {
//   TBenchmark bench;
//   bench.Start("full");
//   //  * Read Tracker APV Data *
//   TString TrackerRawName;
//   vector<TString> datList;
//   if (fileDir.EndsWith(".dat"))
//   {
//     cout << " --> The Input fileName is:" << fileDir << endl;
//     TString PreName;
//     PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
//     datList.push_back(fileDir);
//     cout << " --> The prename is:" << PreName << endl;
//     TrackerRawName = PreName + "-raw.root";
//   }
//   else
//   {
//     if (fileDir.EndsWith("/"))
//       fileDir.Remove(fileDir.Length() - 1, 1);
//     cout << " --> The Input fileDir is:" << fileDir << endl;

//     TString filePath = fileDir + "/Tracker/";
//     GetFileList(filePath, ".dat", datList);

//     TrackerRawName = fileDir + "/Combine/Tracker-step1-raw.root";
//   }

//   // #produce raw-root
//   cout <<endl<<" --> Convert raw data to root, Output : " << TrackerRawName << endl;
//   APV2Root fAPV2Root(datList, TrackerRawName, force);
//   if(fAPV2Root.Init()) fAPV2Root.Loop();
//   cout<<endl;

//   cout<<" --> Read tracker data successfully !"<<endl;

//   bench.Show("full");
// }

// void UnzipTDC(TString fileDir, int force)
// {
//   TBenchmark bench;
//   bench.Start("full");
//   //  * Read DTOF Data *
//   TString DtofRawName;
//   vector<TString> datList;
//   if (fileDir.EndsWith(".dat"))
//   {
//     cout << " --> The Input fileName is:" << fileDir << endl;
//     TString PreName;
//     PreName.Replace(0, 1000, fileDir, fileDir.Last('.'));
//     datList.push_back(fileDir);
//     cout << " --> The prename is:" << PreName << endl;
//     DtofRawName = PreName + "-raw.root";
//   }
//   else
//   {
//     if (fileDir.EndsWith("/"))
//       fileDir.Remove(fileDir.Length() - 1, 1);
//     cout << " --> The Input fileDir is:" << fileDir << endl;

//     TString filePath = fileDir + "/DTOF/";
//     GetFileList(filePath, ".dat", datList);

//     DtofRawName = fileDir + "/Combine/Dtof-step1-raw.root";
//   }

//   // #produce raw-root
//   cout << endl
//        << " --> Convert raw data to root, Output : " << DtofRawName << endl;
//   DtofDat2Root fDtofDat2Root(datList, DtofRawName, force);
//   if (fDtofDat2Root.Init())
//     fDtofDat2Root.Loop();
//   cout << endl;

//   cout << " --> Read dtof data successfully !" << endl;

//   bench.Show("full");
// }
