#include <cstdio>
#include<cmath>
#include "macrostruct.h"

//���dict[i][j]=true���ʾ�����i�����j��֮�����ת�������;
bool dict[3][3]={{false,true,true},{true,false,false},{true,false,false}};

//�Ӷ������ļ���ȡvec���飬����vec����;
void loadvec(FILE* infile,vector<double>& vec )
{
		int size;
		vec.clear();
		fread(&size,sizeof(int),1,infile);//��ȡvec����Ĵ�С;
		if (size==0) 
				return;
		vec.resize(size);//����vec����Ĵ�С;
		fread(&(vec[0]),sizeof(double),size,infile);//��ȡvec���������;
		return ;
}

//��ȡת�������ļ�;
void readmac(macro& mac, FILE* f)
{
		fread(&(mac.ratio),sizeof(double),1,f);//���Ԫ�Ŀ�߱�;
		fread((mac.size),sizeof(double),3,f);//lx,ly,lz;
		fread(&(mac.dzbound),sizeof(double),1,f);//���Ԫ�ĸ߶ȣ�
		loadvec(f,mac.mediums);//�ȵ�������;
		loadvec(f,mac.h);//�ȴ���ϵ������;

		mac.inner=new vector<double> [3];//ת�Ƹ�������;
		for (int i=0;i<3;i++)//��ȡͬһ�ֲ����ڲ�������ת�Ʊ�;
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
	//	mac.bound=new vector<double>[3];//�����߽�ת�������;
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
