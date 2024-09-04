#include "APV2DetStrip.h"

APV2DetStrip::APV2DetStrip(TString encodeName)
{
  //is_list_get = get_encoded_lists(encodeName);
  is_list_get=cal_encoded_lists();
  max_ch_N = max_Nch_use;
}

APV2DetStrip::~APV2DetStrip()
{
}

void APV2DetStrip::clear()
{
  hit_strip.clear();
  hit_amp.clear();
  hit_strip_num.clear();
}

bool APV2DetStrip::decodeHit(vector<int> aget_ch, vector<double> aget_amp)
{
  if (!vectorCheck(aget_ch, aget_amp))
    return false;

  //// decode hit
  if (aget_ch.size() > max_ch_N)
    get_top_hit(aget_ch, aget_amp, max_ch_N);
  if (aget_ch.size() == 1)
  {
    hit_strip.push_back(-aget_ch[0] - 1); // -1 to -64
    hit_amp.push_back(aget_amp[0]);
    hit_strip_num.push_back(1);
    return true;
  }

  vector<int> det_ch;
  vector<double> det_amp;

  for (int i = 0; i < (int)aget_ch.size(); i++)
  {
    det_ch.insert(det_ch.end(), encoded_lists[aget_ch[i]].begin(), encoded_lists[aget_ch[i]].end());
    for (int j = 0; j < (int)encoded_lists[aget_ch[i]].size(); j++)
      det_amp.push_back(aget_amp[i]);
  }
  bubble_sort_ch(det_ch, det_amp);
  vector<int> continuous_num = calc_continous_num(det_ch);
  vector<int> continuous_pos = calc_continous_pos(continuous_num);

  if (continuous_pos.size() == 0)
  {
    get_top_hit(aget_ch, aget_amp, 1);
    hit_strip.push_back(-aget_ch[0] - 1); // -1 to -64
    hit_amp.push_back(aget_amp[0]);
    hit_strip_num.push_back(1);
    // cout << " --> Decode failed, no countinuous num " << endl;
    return true;
  }

  for (int i = 0; i < continuous_pos.size(); i++)
  {
    double chn_total = 0;
    double amp_total = 0;
    int pos_start = continuous_pos[i];
    for (int j = 0; j < continuous_num[pos_start]; j++)
    {
      chn_total += det_ch[pos_start + j] * det_amp[pos_start + j];
      amp_total += det_amp[pos_start + j];
    }
    hit_strip.push_back(chn_total / amp_total + 0.5);
    hit_amp.push_back(amp_total);
    hit_strip_num.push_back(continuous_num[pos_start]);
  }

  result_sort();
  return hit_strip.size();
}

bool APV2DetStrip::get_encoded_lists(TString filename)
{
  ifstream detector_list_file;
  detector_list_file.open(filename.Data());
  string line, chID;
  vector<int> list_array;
  if (detector_list_file.is_open())
  {
    // Read the title line
    getline(detector_list_file, line);
    int count = 0;
    while (getline(detector_list_file, line))
    {
      istringstream line_stream(line);
      while (line_stream)
      {
        if (!getline(line_stream, chID, ',') || chID == "")
          break;
        list_array.push_back(stoi(chID));
      }
      for (int i = 1; i < list_array.size(); i++)
      {
        encoded_lists[list_array[0]].push_back(list_array[i]);
      }
      count++;
      list_array.clear();
    }
    if (count != 64)
    {
      cout << filename << " --> content error!!! total lines: " << count << endl;
      detector_list_file.close();
      return false;
    }
  }
  else
  {
    cout << " --> File " << filename << " open error!!!" << endl;
    detector_list_file.close();
    return false;
  }
  detector_list_file.close();
  /*for(int i=0; i<64; i++)
  {
    cout<<" --> AsicCH = "<<i<<", DetCH = ";
    for(int j=0; j<(int)encoded_lists[i].size(); j++)
      cout<<encoded_lists[i][j]<<" ";
    cout<<endl;
  }*/
  return true;
}

bool APV2DetStrip::cal_encoded_lists()//for simplity , reserve it
{
  for(int i=0;i<256;i++)
    encoded_lists[i].push_back(i);
  /*for(int i=0; i<64; i++)
  {
    cout<<" --> AsicCH = "<<i<<", DetCH = ";
    for(int j=0; j<(int)encoded_lists[i].size(); j++)
      cout<<encoded_lists[i][j]<<" ";
    cout<<endl;
  }*/
  return true;
}

void APV2DetStrip::get_top_hit(vector<int> &ch, vector<double> &amp, int maxN)
{
  if (ch.size() <= maxN)
    return;
  bubble_sort_amp(ch, amp);
  int chn_sz = ch.size();
  for (int i = maxN; i < chn_sz; i++)
  {
    amp.erase(amp.begin() + maxN);
    ch.erase(ch.begin() + maxN);
  }
}

void APV2DetStrip::bubble_sort_amp(vector<int> &ch, vector<double> &amp)
{
  int len = ch.size();
  double tmp;
  for (int i = 0; i < len - 1; i++)
  {
    for (int j = 0; j < len - i - 1; j++)
    {
      if (amp[j] < amp[j + 1])
      {
        tmp = ch[j];
        ch[j] = ch[j + 1];
        ch[j + 1] = tmp;
        tmp = amp[j];
        amp[j] = amp[j + 1];
        amp[j + 1] = tmp;
      }
    } // for j loop
  } // for i loop
}

void APV2DetStrip::bubble_sort_ch(vector<int> &ch, vector<double> &amp)
{
  int len = ch.size();
  double tmp;
  for (int i = 0; i < len - 1; i++)
  {
    for (int j = 0; j < len - i - 1; j++)
    {
      if (ch[j] > ch[j + 1])
      {
        tmp = ch[j];
        ch[j] = ch[j + 1];
        ch[j + 1] = tmp;
        tmp = amp[j];
        amp[j] = amp[j + 1];
        amp[j + 1] = tmp;
      }
    }
  }
}

void APV2DetStrip::result_sort()
{
  if (hit_strip.size() != 1)
  {
    double tmp;
    for (int i = 0; i < hit_strip.size() - 1; i++)
    {
      for (int j = 0; j < hit_strip.size() - i - 1; j++)
      {
        if (hit_strip_num[j] < hit_strip_num[j + 1])
        {
          tmp = hit_strip[j];
          hit_strip[j] = hit_strip[j + 1];
          hit_strip[j + 1] = tmp;
          tmp = hit_amp[j];
          hit_amp[j] = hit_amp[j + 1];
          hit_amp[j + 1] = tmp;
          tmp = hit_strip_num[j];
          hit_strip_num[j] = hit_strip_num[j + 1];
          hit_strip_num[j + 1] = tmp;
        }
      }
    }
  }
}

vector<int> APV2DetStrip::calc_continous_num(vector<int> ch)
{
  vector<int> continuous_num;
  if (ch.size() == 0)
    return continuous_num;

  int len = ch.size();
  for (int i = 0; i < len; i++)
  {
    continuous_num.push_back(1);
    for (int j = i; j < len - 1; j++)
    {
      if (ch[j] + 1 == ch[j + 1])
        continuous_num[i]++;
      else
        break;
    }
  }
  return continuous_num;
}

vector<int> APV2DetStrip::calc_continous_pos(vector<int> continuous_num)
{
  vector<int> continuous_pos;
  if (continuous_num.size() == 0)
    return continuous_pos;
  for (int i = 0; i < continuous_num.size();)
  {
    if (continuous_num[i] == 1)
      i++;
    else
    {
      continuous_pos.push_back(i);
      i += continuous_num[i];
    }
  }
  return continuous_pos;
}

bool APV2DetStrip::vectorCheck(vector<int> aget_ch, vector<double> aget_amp)
{
  if (!is_list_get)
  {
    cout << " --> Hit lists not get please check the code!!! " << endl;
    return false;
  }
  if (aget_ch.size() != aget_amp.size())
  {
    cout << " --> input vector size error, ch_size != amp_size" << endl;
    return false;
  }
  if (aget_ch.size() == 0)
  {
    cout << " --> input vector size = 0" << endl;
    return false;
  }
  if ((*max_element(aget_ch.begin(), aget_ch.end()) >= 255) || (*min_element(aget_ch.begin(), aget_ch.end()) < 0))
  {
    cout << "max: " << *max_element(aget_ch.begin(), aget_ch.end()) << endl;
    cout << "min:" << *min_element(aget_ch.begin(), aget_ch.end()) << endl;
    cout << "Channel reconstrut error" << endl;
    return false;
  }
  return true;
}
