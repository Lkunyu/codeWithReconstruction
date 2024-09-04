
int mergeCali(){
    ifstream inlog1;
    double T0offsetIn1[192];
    inlog1.open("T0channelCalibefore.txt");
    ifstream inlog2;
    double T0offsetIn2[192];
    inlog2.open("T0channelCaliafter.txt");
    ofstream outlog;
    double T0offsetOut[192];
    outlog.open("T0Cali.txt");    
    for(int i=0;i<192;i++){
        inlog1>>T0offsetIn1[i];
        inlog2>>T0offsetIn2[i];
        T0offsetOut[i]=T0offsetIn1[i]+T0offsetIn2[i];
        outlog<<T0offsetOut[i]<<endl;
    }
    cout<<"merge succcess!"<<endl;
    return 0;
}