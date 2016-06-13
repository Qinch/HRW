#ifndef MACROSTRUCT_H__
#define MACROSTRUCT_H__

#include <cstdio>
#include <vector>
#include<cmath>
using std::vector;

struct macro
{
	double ratio; //dx/dz  转移区域内体积元的宽高比dx/dz;
	double size[3]; //(250.5,250.5,250.5);
	double dzbound;//转移区域内体积元的高度;
	vector<double> mediums;//芯片中不同值的热导率。
	vector<double> h;//对流边界不同值的热传递系数。
	vector<double>* inner;//转移区域在同一层芯片内。无论转移区域位于那一层芯片内部，其概率分布都是定值。
	vector<double>*** intersect;//转移区域位于两层芯片内。
	//vector<double>* bound;//转移区域位于对流边界。
};

void readmac(macro& macros, FILE* f);
void loadvec(FILE* infile,vector<double>& vec );
void freemem(macro* mac);

#endif
