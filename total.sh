#########################################################################
# File Name: total.sh
# Author: qinchao 
# mail: 1187620726@qq.com
# Created Time: Date:2015-08-15 Time:08:24:06.
#########################################################################
#!/bin/bash
sum()
{
	if [[ $1 = "m" ]] ;then
		dir="./HRW"
	elif [[ $1 = "b" ]];then
		dir="./GRW"
	elif [[ $1 = "o" ]];then
		dir="./HOPTAB"
	else
		dir="./"
	fi
	find $dir -type f \( -name "*.cpp" -o -name "*.h" \)|xargs wc -l ;
}
sum $1;
