#ifndef MODEL_H_
#define MODEL_H_

#include<cstdio>

struct model
{
    double* x1;//每层芯片的xy坐标范围;
    double* x2;
    double* y1;
    double* y2;
    double* zval;//每层芯片的z轴坐标;
    double* cond;//每层芯片的热导率;
    double hbottom;//最顶层芯片的对流边界的热传递系数;
    double htop;//最底层芯片的对流边界的热传递系数;
    double dxy;//每个体积元的长和宽的大小;
    double* dz;//每层芯片的体积元的高;
    double tout;//环境温度;
    int laynum;//芯片的层数;
};

void read(model* m,FILE* f);
void read(model* m,char* filename);//读取model文件;

#endif /* MODEL_H_ */
