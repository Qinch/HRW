#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "model.h"

#define MODEL_BUFFERSIZE 1024

void read(model* m,FILE* f)
{
    char line[MODEL_BUFFERSIZE];
    int begin=0;
    int end=0;
    while(fgets(line,MODEL_BUFFERSIZE,f))
    {
        if(begin)
        {
            if(strcmp(line,"*END\n")==0)//读取结束;
            {
                end=1;
                break;
            }
            else if(strcmp(line,"\n")==0)//忽略回车;
            {
                continue;
            }
            else if(line[0]=='/' && line[1]=='/')//忽略注释语句;
            {
                continue;
            }
            else if(line[0]=='*')
            {
                char cmd[MODEL_BUFFERSIZE];
                sscanf(line,"%s",cmd);

                if(strcmp(cmd,"*laynum")==0)//芯片层数;
                {
                    sscanf(line,"*laynum %d",&(m->laynum));

                    m->x1=new double[m->laynum];
                    m->x2=new double[m->laynum];
                    m->y1=new double[m->laynum];
                    m->y2=new double[m->laynum];
                    m->zval=new double[m->laynum];
                    m->cond=new double[m->laynum];
                    m->dz=new double[m->laynum];
                }
                else if(strcmp(cmd,"*Lxy")==0)//芯片最大长和宽;
                {
                    double lx,ly;
                    sscanf(line,"*Lxy %lf %lf",&lx,&ly);
                }
                else if(strcmp(cmd,"*x-values")==0)//芯片相对于全局坐标系的xy范围;
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*x-values");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->x1[i]));
                    }
                }
                else if (strcmp(cmd,"*x+values")==0)
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*x+values");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->x2[i]));
                    }
                }
                else if (strcmp(cmd,"*y-values")==0)
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*y-values");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->y1[i]));
                    }
                }
                else if (strcmp(cmd,"*y+values")==0)
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*y+values");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->y2[i]));
                    }
                }
                else if (strcmp(cmd,"*xygridsize")==0)//每层芯片的体积元的xy长度;
                {
                    sscanf(line,"*xygridsize %lf",&(m->dxy));
                }
                else if (strcmp(cmd,"*zgridsize")==0)//每层芯片的体积元的高度;
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*zgridsize");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->dz[i]));
                    }
                }
                else if (strcmp(cmd,"*zvalues")==0)//每层芯片的z轴坐标;
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*zvalues 0");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->zval[i]));
                    }
                }
                else if (strcmp(cmd,"*zparas")==0)//每层芯片的热导率;
                {
                    FILE *stream=fmemopen (line, strlen (line), "r");
                    fscanf(stream,"*zparas");
                    for (int i=0; i< m->laynum ; i++)
                    {
                        fscanf(stream,"%lf",&(m->cond[i]));
                    }
                }
                else if (strcmp(cmd,"*h")==0)//对流边界的热传递系数;
                {
                    sscanf(line,"*h %lf %lf",&(m->hbottom),&(m->htop));
                }
                else if (strcmp(cmd,"*Ta")==0)//环境温度;
                {
                    sscanf(line,"*Ta %lf",&(m->tout));
                }
            }
        }
        else
        {
            if (strcmp(line,"BEGIN\n")==0)//文件读取开始;
            {
                begin=1;
            }
        }

    }
    if (!end)
    {
	fprintf(stderr,"No token '*END\\n' after 'BEGIN' in model file.\n");
        exit(146);
    }
}

void read(model* m,char* filename)
{
    FILE* file;
    if ( (file=fopen(filename,"r")) )
    {
        read(m,file);
        fclose(file);
    }
    else
    {
		fprintf(stderr,"Invalid model file '%s'.\n",filename);
        exit(145);
    }
}
