#########################################################################
# File Name: testall.sh
# Author: qinchao 
# mail: 1187620726@qq.com
# Created Time: Date:2014-11-08 Time:13:17:16.
#########################################################################
#!/bin/bash

rm time1.out
touch time1.out
{ time ../bin/frw -m ../data/model1_1.in -p ../data/powermap1.in  -t ../macros/table10.250.1e5.in -n 200 -a > m1_1.out ; } 2>> time1.out
#{ time ../bin/frw -m ../data/model1_2.in -p ../data/powermap1.in  -t ../macros/table10.250.5e6.in -n 200 -a > m1_2.out ; } 2>> time1.out
#{ time ../bin/frw -m ../data/model1_3.in -p ../data/powermap1.in  -t ../macros/table10.250.5e6.in -n 200 -a > m1_3.out ; } 2>> time1.out
