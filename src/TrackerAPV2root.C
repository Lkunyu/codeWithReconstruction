#include "CRInfo.h"
#include "TrackerAPV2root.h"

TrackerAPV2root::TrackerAPV2root() { ; }

TrackerAPV2root::TrackerAPV2root(vector<TString> fList, TString fRawName, int force)
{
    datList = fList;
    rawName = fRawName;
}

TrackerAPV2root::~TrackerAPV2root()
{
}

bool TrackerAPV2root::Init()
{
    TrackerRawFile = new TFile(rawName, "RECREATE");
    TrackerRawTree = new TTree("TrackRaw", "TrackRaw");
    TrackerRawTree->Branch("event", &eventall, "event/I");
    TrackerRawTree->Branch("board", &board, "board/I");
    TrackerRawTree->Branch("chip", &chip, "chip/I");
    TrackerRawTree->Branch("channel", &channel, "channel/I");
    TrackerRawTree->Branch("wave", &wave);

    return true;
}

void TrackerAPV2root::BlankData()
{
    board = -1;
    chip = -1;
    channel = -1;
    wave.clear();
}

void TrackerAPV2root::Loop()
{
    int length = sizeof(short);
    unsigned short memblock;
    int counting = 0;
    int badNumber = 0;
    int event = -1;
    int preEvent = 0;
    int NumEvent = 0;
    long long timeStamp = 0;
    int ReadBoard = 0;
    int RealChip = 0;
    int threshold = 500;
    int datapoint = 0;
    int eventloop[2] = {0, 0};
    bool start_flag = false;

    if (datList.size() == 0)
    {
        cout << "-->file dir is empty, please check..." << endl;
        return;
    }

    vector<struct datastruct> dataall[2];
    struct datastruct datatemp;

    for (int i = 0; i < (int)datList.size(); i++)
    {
        start_flag = false;
        fstream InDat;
        InDat.open(datList[i], ios::in | ios::binary);
        if (!InDat.is_open())
        {
            cout << " ERROR! " << datList[i] << " cant open!" << endl;
            continue;
        }
        cout << " --> Reading a new dat file-" << i << " : " << datList[i] << endl;
        // op << " --> Reading a new dat file-" << i << " : " << datList[i] << endl;
        int readNum = 0;
        unsigned short startblock[3];
        while (!InDat.eof())
        {
            if (start_flag == false)
            {
                for (int ii = 0; ii < 3; ii++)
                {
                    InDat.read((char *)(&memblock), length / 2);
                    memblock <<= 8;
                    memblock >>= 8;
                    startblock[ii] = memblock;
                    cout << "reading ..." << hex << memblock << dec << endl;
                    readNum++;
                }
                do
                {
                    InDat.read((char *)(&memblock), length / 2); // char*位数和unsigned short相同，但我们只读半位，可能导致前两位不变
                    startblock[0] = startblock[1];
                    startblock[1] = startblock[2];
                    startblock[2] = memblock;
                    // cout<<hex<<"block "<<startblock[0]<<" "<<startblock[1]<<" "<<startblock[2]<<endl;
                    readNum++;

                } while ((!((startblock[0] == 0x00CA) && (startblock[1] == 0x00CA) && (startblock[2] == 0x00CA))) && (!InDat.eof()));
                start_flag = true;
                cout << readNum << endl;
                cout << hex << "block " << startblock[0] << " " << startblock[1] << " " << startblock[2] << dec << endl;
            }

            //////////////////////////////////////
            InDat.read((char *)(&memblock), length);
            memblock = exchangehighlow(memblock);
            if (memblock == 0xEEEE)
            {
                if (counting == 9)
                {
                    cout << "empty package" << endl;
                    eventall = event + eventloop[i] * 65536;
                    BlankData();
                    FillStruct(datatemp);
                    dataall[i].push_back(datatemp);
                    // TrackerRawTree->Fill();
                }

                InDat.read((char *)(&memblock), length);
                // cout << std::hex << memblock << ' ';
                chip = (((memblock & 0x0f00))); // 注意，此时memblock的顺序是反的
                RealChip = chip;

                counting = 0;
                datapoint = 0;
                wave.clear();
            }
            else if (memblock == 0xEE5A)
            {
                if (counting == 9)
                {
                    cout << "empty package" << endl;
                    eventall = event + eventloop[i] * 65536;
                    BlankData();
                    FillStruct(datatemp);
                    dataall[i].push_back(datatemp);
                    // TrackerRawTree->Fill();
                }

                InDat.read((char *)(&memblock), length / 2); // 此时读取的十六位数会放在memblock的后十六位
                // cout << std::hex << memblock << ' ';
                chip = (((memblock & 0x000f))); // 注意，此时memblock的顺序是反的
                RealChip = chip;
                counting = 0;
                datapoint = 0;
                wave.clear();
            }
            else if (counting == 4)
            {

                preEvent = event;
                event = memblock;
                if ((event - preEvent < -65000) && preEvent > 65000)
                {
                    eventloop[i]++;
                    cout << "new loop! loop = " << eventloop[i] << endl;
                }
                eventall = event + eventloop[i] * 65536;
            }
            else if ((counting == 9) && ((memblock & 0xff00) == 0x0000))
            {
                bool setup = 0;
                if (setup == 0)
                {
                    if (i == 0)
                    {
                        if (RealChip <= 3)
                        {
                            board = 0;
                            chip = RealChip;
                        }
                        else if (RealChip >= 8)
                        {
                            board = 1;
                            chip = RealChip - 8;
                        }
                    }

                    else if (i == 1)
                    {
                        if (RealChip <= 3)
                        {
                            board = 2;
                            // chip = RealChip;
                            switch (RealChip)
                            {
                            case 0:
                                chip = 2;
                                break;
                            case 1:
                                chip = 3;
                                break;
                            case 2:
                                chip = 0;
                                break;
                            case 3:
                                chip = 1;
                                break;
                            default:
                                break;
                            }
                        }
                        else if (RealChip >= 8)
                        {
                            // cout<<"RealChip "<<RealChip<<endl;
                            board = 3;
                            // chip = RealChip - 8;
                            switch (RealChip)
                            {
                            case 8:
                                chip = 2;
                                break;
                            case 9:
                                chip = 3;
                                break;
                            case 10:
                                chip = 0;
                                break;
                            case 11:
                                chip = 1;
                                break;
                            default:
                                break;
                            }
                        }
                        else if (RealChip > 3 && RealChip < 8)
                        {
                            chip = -1;
                            continue;
                        }
                    }
                }
                else if (setup == 1)
                {
                    if (i == 0)
                    {
                        if (RealChip <= 3)
                        {
                            board = 1;
                            // chip = RealChip;
                            switch (RealChip)
                            {
                            case 0:
                                chip = 2;
                                break;
                            case 1:
                                chip = 3;
                                break;
                            case 2:
                                chip = 0;
                                break;
                            case 3:
                                chip = 1;
                                break;
                            default:
                                break;
                            }
                        }
                        else if (RealChip >= 8)
                        {
                            board = 0;
                            chip = RealChip - 8;
                        }
                    }

                    else if (i == 1)
                    {
                        if (RealChip <= 3)
                        {
                            board = 2;
                            chip = RealChip;
                        }
                        else if (RealChip >= 8)
                        {
                            board = 3;
                            chip = RealChip - 8;
                        }
                    }
                }

                channel = memblock & 0x00ff;
                // std::cout << "channel number = " << channel << endl;
            }
            else if (counting == 40)
            {
                if ((memblock & 0xff00) == 0x0000)
                {
                    channel = memblock & 0x00ff;
                    // std::cout << "channel number = " << channel << endl;
                    // counting = 7;
                    counting = 9;
                }

                if ((memblock & 0xff00) == 0xCA00)
                {
                    counting = 0;
                    // cout << " event: " << event << " chip: " << chip << endl;
                    // if (memblock == 0xCACA)
                    //{
                    //    event++;
                    //}
                }
            }
            else if ((counting >= 10) && (counting <= 39))
            {

                short buffer = memblock & 0x0fff;
                if (buffer >= 0x0800)
                {
                    // cout << "buffer = " << buffer << "  buffer-0xfff= " << buffer - 0xfff << endl;
                    buffer = buffer - 0xfff;
                }

                wave.push_back(-buffer);

                if (counting == 39)
                {
                    if (chip == -1)
                        continue;
                    if ((wave.size() == 30))
                    {
                        int max = 0;

                        if (board < 0 || board > 3 || chip < 0 || chip > 3 || channel < 0 || channel > 127)
                        {
                            cout << "error: board or chip or channel out of range" << endl;
                            cout << " counting: " << counting << " event: " << event << " board: " << board << " chip: " << chip << " channel: " << channel << " wave size: " << wave.size() << endl;
                            continue;
                        }
                        FillStruct(datatemp);
                        dataall[i].push_back(datatemp);
                        // TrackerRawTree->Fill();
                    }
                    else if (wave.size() != 0)
                    {
                        cout << " wave size not equal to 30, bad wave. " << " counting: " << counting << " event: " << event << " board: " << board << " chip: " << chip << " channel: " << channel << " wave size: " << wave.size() << endl;
                        badNumber++;
                    }
                    wave.clear();
                }
            }
            counting++;
            datapoint++;
        }
    }
    int eventbegin = 65536 * 512;
    int eventend = -65536;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < dataall[i].size(); j++)
        {
            if (eventbegin > dataall[i].at(j).eventevent)
            {
                eventbegin = dataall[i].at(j).eventevent;
            }
            if (eventend < dataall[i].at(j).eventevent)
            {
                eventend = dataall[i].at(j).eventevent;
            }
        }
    }
    cout << "-->From begin event " << eventbegin << " to " << eventend << " Filling root data..." << endl;
    int filepos[2] = {0, 0};
    int pos = 0;
    for (eventall = eventbegin; eventall < eventend; eventall++)
    {
        DrawProcessbar(eventall, eventend - eventbegin);
        for (int j = 0; j < 2; j++) // 2 files means 2 detectors
        {
            while ((filepos[j] < dataall[j].size()) && (dataall[j].at(filepos[j]).eventevent <= eventall))
            {
                if (dataall[j].at(filepos[j]).eventevent == eventall)
                {
                    StructOut(dataall[j].at(filepos[j]));
                    // cout<<"board "<<board<<" and chip "<<chip<<endl;
                    TrackerRawTree->Fill();
                }
                filepos[j]++;
            }
        }
    }

    cout << "--> Total entries = " << TrackerRawTree->GetEntries() << endl;
    cout << "--> Bad Events = " << badNumber << endl;
    cout << endl;

    cout << "--> Track-APV Raw data file has been convert to root-file: " << rawName << ".\n"
         << endl;
    TrackerRawFile->WriteTObject(TrackerRawTree);
    TrackerRawFile->Close();
}

unsigned short TrackerAPV2root::exchangehighlow(unsigned short data)
{

    unsigned short low = data & 0x00ff;
    unsigned short high = (data >> 8) & 0x00ff;
    unsigned short aa = low;
    aa <<= 8;
    aa |= high;
    return aa;
}