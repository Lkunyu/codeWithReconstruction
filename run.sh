#!/bin/bash
make

for i in {1..5}; do
    ./ReadTracker ../../BeamData/run016$i 1
done

root -l ../openTBrowser.C
