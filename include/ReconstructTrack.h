#ifndef RECONSTRUCTTRACK_h
#define RECONSTRUCTTRACK_h 1

#include "CRInfo.h"

const int MMdetN = 4;
class ReconstructTrack
{
public:
  TFile *fdecFile;
  TTree *fdecTree[MMdetN];
  TFile *frecFile;
  TTree *frecTree;
  ReconstructTrack(TString fdecName, TString frecName, int force);
  ~ReconstructTrack();
  bool Init();
  void StoreCluster(int n);
  void Loop();
  void InitData();

  /// for efficiency cut
  double theta_cut = 1; // degree
  double pos_cut = 40;  // mm

  //// track selection, correction, reconstruction
  bool is_single_correct = true;   // if correct single hit
  double centre = 256.0 / 2 * 0.4; // layer centre, mm
  double fit_R2_cut = 0.995;       // correlation index squre
  bool fit_track(vector<double> px, vector<double> py, double par[3]);
  double hit_reconstruction(vector<int> id, vector<double> px, vector<double> &py, int xy);
  double hit_selection(int eraseN, vector<int> id, vector<double> px, vector<double> &py, int xy);
  bool track_reconstruction(vector<double> px, vector<double> py, vector<double> pz);

  /// asic channel to detector channel
  vector<int> APV2det[256];
  bool get_APV2det_lists(string lists_name);
  bool cal_APV2det_lists();
  /// detector alignment
  double alg_fit_R2_cut = 0.95;              // correlation index squre
  double alg_theta_cut = 5;                  // degree
  double alg_fit_res_cut = 2;                // mm, 3-4 sigma is better
  double alg_res = 5e-8;                     // delta-residual resolution
  double alg_rho = 0.95, alg_epsilon = 1e-4; // for alginment loop
  vector<string> ParName = {"deltaX", "deltaY", "deltaZ", "thetaX", "thetaY", "thetaZ"};
  double algPar3[MMdetN][6];
  double algPar6[MMdetN][6];
  TString alignmentPar3Name;
  TString alignmentPar6Name;
  void ReadAlignmentPars(string filename, int Npars = 6);
  void Alignment(string filename, int Npars = 6);

private:
  TString decName;
  TString recName;
  int force;
  /// input data
  int event;
  bool sig;
  bool sig_x;
  bool sig_y;
  double x;
  double y;
  double z;
  double hit_amp_x;
  double hit_amp_y;
  int hit_strip_num_x;
  int hit_strip_num_y;
  int x_nhits;
  int y_nhits;
  vector<double> *vec_Cluster_x = 0;
  vector<double> *vec_Cluster_y = 0;
  vector<double> *x_other = 0;
  vector<double> *y_other = 0;
  TBranch *b_event;
  TBranch *b_sig;
  TBranch *b_sig_x;
  TBranch *b_sig_y;
  TBranch *b_x;
  TBranch *b_y;
  TBranch *b_z;
  TBranch *b_hit_amp_x;
  TBranch *b_hit_amp_y;
  TBranch *b_hit_strip_num_x;
  TBranch *b_hit_strip_num_y;
  TBranch *b_x_nhits;
  TBranch *b_y_nhits;
  TBranch *b_x_other;
  TBranch *b_y_other;
  TBranch *b_vec_Cluster_x;
  TBranch *b_vec_Cluster_y;
  /// output data
  vector<vector<double>> cluster_x;
  vector<vector<double>> cluster_y;
  double MMx[MMdetN];
  double MMy[MMdetN];
  double MMz[MMdetN];
  bool MMsigx[MMdetN];
  bool MMsigy[MMdetN];
  double XfitPars[3];
  double YfitPars[3];
  bool useflag;
};

#endif
