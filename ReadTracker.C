#include "CRAnalysis.h"

void show_info()
{
    cout << "Usage: " << endl
         << " Argument0 - data path (required)" << endl
         << " Argument1 - force (recreate outfile ? default 0)" << endl
         << endl;
}
int main(int argc, char *argv[])
{
    if (argc == 2) 
    {
        ReadTracker(argv[1]);
    }

    else if (argc == 3)
    {
        ReadTracker(argv[1], stoi(argv[2]));
    }

    else
        show_info();
    return 0;
}