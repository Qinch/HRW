rm time2.out
touch time2.out
{ time ../bin/frw -m ../data/model2_1.in -p ../data/powermap2.in  -t ../macros/table10.250.5e6.in -n 200 -a > m2_1.out ; } 2>> time2.out
{ time ../bin/frw -m ../data/model2_2.in -p ../data/powermap2.in  -t ../macros/table10.250.5e6.in -n 200 -a > m2_2.out ; } 2>> time2.out
{ time ../bin/frw -m ../data/model2_3.in -p ../data/powermap2.in  -t ../macros/table10.250.5e6.in -n 200 -a > m2_3.out ; } 2>> time2.out
