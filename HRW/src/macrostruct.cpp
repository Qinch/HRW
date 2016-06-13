#include <cstdio>
#include<cmath>
#include "macrostruct.h"

//如果dict[i][j]=true则表示两层第i层与第j层之间存在转移区域表;
bool dict[3][3]={{false,true,true},{true,false,false},{true,false,false}};

//从二进制文件读取vec数组，创建vec数组;
void loadvec(FILE* infile,vector<double>& vec )
{
		int size;
		vec.clear();
		fread(&size,sizeof(int),1,infile);//读取vec数组的大小;
		if (size==0) 
				return;
		vec.resize(size);//设置vec数组的大小;
		fread(&(vec[0]),sizeof(double),size,infile);//读取vec数组的内容;
		return ;
}

//读取转移区域文件;
void readmac(macro& mac, FILE* f)
{
		fread(&(mac.ratio),sizeof(double),1,f);//体积元的宽高比;
		fread((mac.size),sizeof(double),3,f);//lx,ly,lz;
		fread(&(mac.dzbound),sizeof(double),1,f);//体积元的高度；
		loadvec(f,mac.mediums);//热导率数组;
		loadvec(f,mac.h);//热传递系数数组;

		mac.inner=new vector<double> [3];//转移概率数组;
		for (int i=0;i<3;i++)//读取同一种材料内部的区域转移表;
		{
				loadvec(f,mac.inner[i]);
		}
		
		mac.intersect=new vector<double>**[3];
		for(int i=0;i<3;i++)
		{
			mac.intersect[i]=new vector<double>*[3];
			for(int j=0;j<3;j++)
			{
				if(dict[i][j]==true)
				{
					mac.intersect[i][j]=new vector<double>[3];
					for(int k=0;k<3;k++)
							loadvec(f,mac.intersect[i][j][k]);
				}
			}
		}
	//	mac.bound=new vector<double>[3];//对流边界转移区域表;
	//	for (int k=0;k<3;k++)
	//	{
	//		loadvec(f,mac.bound[k]);
	//	}
}

void freemem(macro *mac)
{
	delete[] mac->inner;
	int msize=mac->mediums.size();
	for(int i=0;i<msize;i++)
	{
			for(int j=0;j<msize;j++)
				if(mac->intersect[i][j]!=NULL)
					delete[] mac->intersect[i][j];
			delete[] mac->intersect[i];
	}		
//	delete[] mac->bound;
}
