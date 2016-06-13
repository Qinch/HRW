#ifndef GRID_H_
#define GRID_H_

#include "model.h"

struct grid
{
	struct model m;
	double ***p;//存储每层芯片xyz的概率
	double ***p1;
	double ***p2;
	double* powerfactor;
	double angfactor;
	double facfactor;

	double*** power;
	double* prop;//每层芯片的体积元的宽高比dx/dz
	int* zval;
	int *minx,*miny;
	int *maxx,*maxy;
	int maxz;
	int maxxx,maxyy;
};

void computeprob(grid*g);//计算每层芯片的hop概率
void buildgrid(grid* g);//对struct gird的数据成员进行初始化。
void readpowermap(grid* g,FILE* f);
void readpowermap(grid* g,char* filename);
void freemem(grid *g);//释放堆内存;
void freemem(model *m);//释放内存;

#endif /* GRID_H_ */
