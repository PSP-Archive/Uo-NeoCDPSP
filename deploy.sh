PSPPATH="/cygdrive/n/PSP/GAME/"
mkdir .psp 
cd .psp 
../unpack-pbp ../EBOOT.PBP 
mkdir $PSPPATH$1 
mv UNKNOWN.PSP $PSPPATH$1/EBOOT.PBP 
mkdir $PSPPATH"$1%" 
touch UNKNOWN.PSP 
cp ../data/pic1.png ./PIC1.PNG
cp ../data/icon0.png ./ICON0.PNG
cp ../data/snd0.at3.wav ./SND0.AT3
../_pack-pbp $PSPPATH"$1%/EBOOT.PBP" PARAM.SFO ICON0.PNG ICON1.PMF UKNOWN.PNG PIC1.PNG SND0.AT3 UNKNOWN.PSP UNKNOWN.PSAR 
touch -t 200101010000 $PSPPATH"$1"
touch  $PSPPATH"$1%"
#mv $1* .. 
cd .. 
rm -rf .psp 
