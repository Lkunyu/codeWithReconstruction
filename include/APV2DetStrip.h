#ifndef APV2DETSTRIP_h
#define APV2DETSTRIP_h 1

#include "CRInfo.h"

class APV2DetStrip
{
  public:
    APV2DetStrip(TString encodeName);
    ~APV2DetStrip();
    void clear();
    bool decodeHit(vector<int> aget_ch, vector<double> aget_amp);

    vector<double> hit_strip;
    vector<double> hit_amp;
    vector<int> hit_strip_num;

  private:
    bool get_encoded_lists(TString filename);
    bool cal_encoded_lists();
    bool vectorCheck(vector<int> aget_ch, vector<double> aget_amp);
    void get_top_hit(vector<int>& ch, vector<double>& amp, int maxN);
    vector<int> calc_continous_num(vector<int> ch);
    vector<int> calc_continous_pos(vector<int> continuous_num);
    void result_sort();
    void bubble_sort_amp(vector<int>& ch, vector<double>& amp);
    void bubble_sort_ch(vector<int>& ch, vector<double>& amp);

    int max_ch_N = 59; // AGET to DET MindeltaCH = 60
    array<vector<int>, 256> encoded_lists; 
    bool is_list_get = false;

};

#endif
