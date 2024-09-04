echo "processing in fileDir : $1 "
#dir="/mnt/d/beamtest/data/";
dir=$1

filelist=`ls $dir`

if [ ! -d "$dir/DTOF" ]; then
    mkdir $dir/DTOF
    mkdir $dir/Combine
    mkdir $dir/Tracker
fi
for file in $filelist
do
    if [ "${file##*.}"x = "dat"x ]; then
    echo "moving file: $dir/$file to :$dir/DTOF"
    cp -r $dir/$file $dir/DTOF/
    rm -rf $dir/$file
    fi
done
#processing data

make
./ReadData $dir/ 1
root -l openTBrowser.C


