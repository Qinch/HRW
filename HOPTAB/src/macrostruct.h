#ifndef MACROSTRUCT_H__
#define MACROSTRUCT_H__

#include <cstdio>
#include <vector>
#include"random.h"
using std::vector;

struct Macros
{
	double ratio; //dx/dz  ת�����������Ԫ�Ŀ�߱�dx/dz;
	double size[3]; //(250.5,250.5,250.5);
	double dzbound;//ת�����������Ԫ�ĸ߶�;
	vector<double> mediums;//оƬ�в�ֵͬ���ȵ��ʡ�
	vector<double> h;//�����߽粻ֵͬ���ȴ���ϵ����
	vector<double>* inner;//ת��������ͬһ��оƬ�ڡ�����ת������λ����һ��оƬ�ڲ�������ʷֲ����Ƕ�ֵ��
	vector<double>*** intersect;//ת������λ������оƬ�ڡ�
	vector<double>* bound;//ת������λ�ڶ����߽硣
};

int Pow(int x,int n);
void read(Macros& macros, FILE* f);
void write(Macros& macros, FILE* f);
void init(Macros& macros);

void walk(int walks,Macros& macros,Random &rand);
void walkconv(int walks,vector<double>* conv,Macros& macros,int idx1,int idx2,Random &rand);//ֻ�洢��������棬���洢�����߽�;

#endif
