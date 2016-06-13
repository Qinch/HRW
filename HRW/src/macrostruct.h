#ifndef MACROSTRUCT_H__
#define MACROSTRUCT_H__

#include <cstdio>
#include <vector>
#include<cmath>
using std::vector;

struct macro
{
	double ratio; //dx/dz  ת�����������Ԫ�Ŀ�߱�dx/dz;
	double size[3]; //(250.5,250.5,250.5);
	double dzbound;//ת�����������Ԫ�ĸ߶�;
	vector<double> mediums;//оƬ�в�ֵͬ���ȵ��ʡ�
	vector<double> h;//�����߽粻ֵͬ���ȴ���ϵ����
	vector<double>* inner;//ת��������ͬһ��оƬ�ڡ�����ת������λ����һ��оƬ�ڲ�������ʷֲ����Ƕ�ֵ��
	vector<double>*** intersect;//ת������λ������оƬ�ڡ�
	//vector<double>* bound;//ת������λ�ڶ����߽硣
};

void readmac(macro& macros, FILE* f);
void loadvec(FILE* infile,vector<double>& vec );
void freemem(macro* mac);

#endif
