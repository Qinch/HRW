#include <cstdio>
#include<cmath>
#include <cstring>
#include <cstdlib>
#include "grid.h"
#include "argsparase.h"

#define zvalue(n) (n>=0?(g->zval[n]):0)
#define mzvalue(n) (n>=0? (g->m.zval[n]):0)//第i层芯片的z轴坐标值;
#define minval(x,y) (x>y?y:x)
#define maxval(x,y) (x>y?x:y)

#define GRID_BUFFERSIZE 1024
#define EPSILON 1e-8
#define ROUND 0.5

void computeprob(grid*g)//计算每层芯片的xyz概率;
{
	g->p1=new double**[g->m.laynum];//角;
	g->p2=new double**[g->m.laynum];//面;
	g->p=new double**[g->m.laynum];//内;

	for(int i=0; i<g->m.laynum; i++)
	{
		//g->p1表示每层芯片角的hop概率;
		//g->p2表示每层芯片面的hop概率;
		//g->p表示每层芯片内部的hog->p概率;
		g->p1[i]=new double*[3];
		g->p2[i]=new double*[3];
		for(int j=0; j<3; j++)
		{
			g->p1[i][j]=new double[2];
			g->p2[i][j]=new double[2];
		}

		if( (i==0) || (fabs(g->m.x1[i]-g->m.x1[i-1])<EPSILON) )//当前层为第0层或者下层芯片和当前层芯片长宽相同的情况;
		{
			g->p[i]=new double*[3];
			for(int j=0; j<3; j++)
			{
				g->p[i][j]=new double[2];
			}
		}
		else //下层芯片长宽小于当前层芯片的情况;
		{
			g->p[i]=new double*[4];
			for(int j=0; j<4; j++)
			{
				g->p[i][j]=new double[2];//g->p[i][3]对应内下特(即两层芯片的非接触面);
			}
		}

		double Gx,Gz1,Gz2;//热导率;
		double sub;//分母;
		//下;
		Gx=g->m.cond[i]*g->m.dz[i];
		//Gz的3种情况;
		double temp2=1/( g->m.dz[i]/2/g->m.cond[i]/g->m.dxy/g->m.dxy +     //上;
									g->m.dz[i+1]/2/g->m.cond[i+1]/g->m.dxy/g->m.dxy );
		double temp1=g->m.cond[i]*g->m.dxy*g->m.dxy/g->m.dz[i];           //中;
		double temp0=1/( g->m.dz[i]/2/g->m.cond[i]/g->m.dxy/g->m.dxy+      //下;
									g->m.dz[i-1]/2/g->m.cond[i-1]/g->m.dxy/g->m.dxy );
		int interval=(mzvalue(i)-mzvalue(i-1))/g->m.dz[i]+ROUND;//当前层芯片划分为interval层体积元;

		if( (i==0) || (fabs(g->m.x1[i-1]-g->m.x1[i])>EPSILON) )//两层芯片不接触的面，当前层芯片的长宽大于下层芯片的长宽;
		{
			Gz1=temp1;
			sub=Gx*2+Gz1;//角下;
			g->p1[i][0][0]=Gx/sub;//x,y方向的转移概率;
			g->p1[i][0][1]=Gz1/sub;

			sub=Gx*3+Gz1;//面下;
			g->p2[i][0][0]=Gx/sub;
			g->p2[i][0][1]=Gz1/sub;

			int temp;
			if(i==0)
			{
				temp=0;
			}
			else
				temp=3;//对应 当前层的长宽大于下层长宽的情况（当前层下表面与下层上表面不接触的区域的概率）;
			sub=Gx*4+Gz1;//内下(特殊情况)
			g->p[i][temp][0]=Gx/sub;
			g->p[i][temp][1]=Gz1/sub;
		
			if(i!=0)//两层芯片接触的面;
			{
				Gz2=temp0;
				sub=Gx*4+Gz1+Gz2;
				g->p[i][0][0]=Gx/sub;
				g->p[i][0][1]=Gz1/sub;
			}	    	
		}
		else //当前层长宽等于下层长宽的情况;
		{
			if(interval==1)
			{
				Gz2=temp0;
				Gz1=temp2;
			}
			else
			{
				Gz1=temp1;
				Gz2=temp0;
			}

			sub=Gz1+Gz2+Gx*2;//角下;
			g->p1[i][0][0]=Gx/sub;
			g->p1[i][0][1]=Gz1/sub;

			sub=3*Gx+Gz1+Gz2;//面下;
			g->p2[i][0][0]=Gx/sub;
			g->p2[i][0][1]=Gz1/sub;

			sub=Gz1+Gz2+4*Gx;//内下,存储的两层芯片的接触面的情况;
			g->p[i][0][0]=Gx/sub;
			g->p[i][0][1]=Gz1/sub;
		}

		if(interval==1)
		{
			Gz2=temp0;
			Gz1=temp2;
		}
		else
		{
			Gz1=temp1;
			Gz2=temp1;
		}
		//中;
		sub=Gx*2+Gz1+Gz2;
		g->p1[i][1][0]=Gx/sub;//角中;
		g->p1[i][1][1]=Gz1/sub;

		sub=3*Gx+Gz2+Gz1;//面中;
		g->p2[i][1][0]=Gx/sub;
		g->p2[i][1][1]=Gz1/sub;

		sub=Gx*4+Gz2+Gz1;//内中;
		g->p[i][1][0]=Gx/sub;
		g->p[i][1][1]=Gz1/sub;

		//上;
		if(i==g->m.laynum-1)//对流边界只进行z方向上的行走;
		{
			Gz1=temp1;
			Gz2=g->m.htop*g->m.dxy*g->m.dxy;
			sub=Gz1+Gz2;
			g->p1[i][2][0]=0;//对流边界至进行z方向的行走;
			g->p1[i][2][1]=Gz1/sub;//角上;

			g->p2[i][2][0]=0;//面上;
			g->p2[i][2][1]=g->p1[i][2][1];

			g->p[i][2][0]=0;//内上;
			g->p[i][2][1]=g->p1[i][2][1];
		}
		else
		{
			if(interval==1)
			{
				Gz1=temp0;
				Gz2=temp2;
			}
			else
			{
				Gz1=temp1;
				Gz2=temp2;
			}
			sub=2*Gx+Gz1+Gz2;//角上;
			g->p1[i][2][0]=Gx/sub;
			g->p1[i][2][1]=Gz1/sub;

			sub=3*Gx+Gz1+Gz2;
			g->p2[i][2][0]=Gx/sub;//面上;
			g->p2[i][2][1]=Gz1/sub;

			sub=Gz1+Gz2+4*Gx;//内上;
			g->p[i][2][0]=Gx/sub;
			g->p[i][2][1]=Gz1/sub;
		}
	}
    //输出计算结果(用于验证);
    /*FILE* fp=fopen("prob.txt","w");
    for(int i=0;i<g->m.laynum;i++)
    {
        fprintf(fp,"layer:%d\n",i);
        if(i==0||g->m.x1[i]==g->m.x1[i-1])
        {
            fprintf(fp,"p1角bottom: x %lf, z %lf\n",g->p1[i][0][0],g->p1[i][0][1]);
            fprintf(fp,"p1角middle: x %lf, z %lf\n",g->p1[i][1][0],g->p1[i][1][1]);
            fprintf(fp,"p1角top   : x %lf, z %lf\n",g->p1[i][2][0],g->p1[i][2][1]);

            fprintf(fp,"p2面bottom: x %lf, z %lf\n",g->p2[i][0][0],g->p2[i][0][1]);
            fprintf(fp,"p2面middle: x %lf, z %lf\n",g->p2[i][1][0],g->p2[i][1][1]);
            fprintf(fp,"p2面top   : x %lf, z %lf\n",g->p2[i][2][0],g->p2[i][2][1]);

            fprintf(fp,"p内bottom: x %lf, z %lf\n",g->p[i][0][0],g->p[i][0][1]);
            fprintf(fp,"p内middle: x %lf, z %lf\n",g->p[i][1][0],g->p[i][1][1]);
            fprintf(fp,"p内top   : x %lf, z %lf\n",g->p[i][2][0],g->p[i][2][1]);
            fprintf(fp,"---------------------------------------------------\n");

        }
        else{
            fprintf(fp,"p1角bottom: x %lf, z %lf\n",g->p1[i][0][0],g->p1[i][0][1]);
            fprintf(fp,"p1角middle: x %lf, z %lf\n",g->p1[i][1][0],g->p1[i][1][1]);
            fprintf(fp,"p1角top   : x %lf, z %lf\n",g->p1[i][2][0],g->p1[i][2][1]);

            fprintf(fp,"p2面bottom: x %lf, z %lf\n",g->p2[i][0][0],g->p2[i][0][1]);
            fprintf(fp,"p2面middle: x %lf, z %lf\n",g->p2[i][1][0],g->p2[i][1][1]);
            fprintf(fp,"p2面top   : x %lf, z %lf\n",g->p2[i][2][0],g->p2[i][2][1]);

            fprintf(fp,"p内bottom: x %lf, z %lf\n",g->p[i][0][0],g->p[i][0][1]);
            fprintf(fp,"p内middle: x %lf, z %lf\n",g->p[i][1][0],g->p[i][1][1]);
            fprintf(fp,"p内top   : x %lf, z %lf\n",g->p[i][2][0],g->p[i][2][1]);
            fprintf(fp,"p内bottom(2): x %lf, z %lf\n",g->p[i][3][0],g->p[i][3][1]);//存储的是两层芯片的非接触面
            fprintf(fp,"---------------------------------------------------\n");
        }
    }
    fclose(fp);*/
}

void buildgrid(grid* g)
{
	computeprob(g);//计算每层芯片的概率;

	g->prop=new double[g->m.laynum];//计算每层芯片的dx/dz;
	for (int i=0; i<g->m.laynum; i++)
	{
		g->prop[i]=(g->m.dxy)/(g->m.dz[i]);
	}

	g->zval=new int [g->m.laynum];//存储从第0层芯片到当前层的体积元的总层数;
	for (int i=0; i<g->m.laynum; i++)
	{
		g->zval[i]=(i==0?0:g->zval[i-1])+(mzvalue(i)-mzvalue(i-1))/g->m.dz[i]+ROUND;//从第0层芯片到第i层芯片的体积元层数;
	}
	g->maxz=g->zval[g->m.laynum-1];//一共有多少层体积元;

	int xmax=0,ymax=0;
	g->minx=new int[g->m.laynum];
	g->miny=new int[g->m.laynum];
	g->maxx=new int[g->m.laynum];
	g->maxy=new int[g->m.laynum];
	for (int i=0; i<g->m.laynum; i++)
	{
		g->minx[i]=g->m.x1[i]/(g->m.dxy)+ROUND;
		g->miny[i]=g->m.y1[i]/(g->m.dxy)+ROUND;
		g->maxx[i]=g->m.x2[i]/(g->m.dxy)+ROUND;
		g->maxy[i]=g->m.y2[i]/(g->m.dxy)+ROUND;
		if (g->maxx[i]>xmax)
			xmax=g->maxx[i];
		if (g->maxy[i]>ymax)
			ymax=g->maxy[i];
	}

	g->power=new double** [g->maxz];
	for (int i=0; i<g->maxz; i++)
	{
		g->power[i]=NULL;
	}

	g->powerfactor=new double [g->m.laynum];
	for (int i=1;i<2;i++)//已知power层即第一层芯片，并且power层只划分一层体积元;
	{
		g->powerfactor[i]=0;
		double Gz1,Gz2,Gx;
		Gx=g->m.cond[i]*g->m.dz[i];
		Gz1=1/(g->m.dz[i-1]/2/g->m.cond[i-1]/g->m.dxy/g->m.dxy + g->m.dz[i]/2/g->m.cond[i]/g->m.dxy/g->m.dxy);
		Gz2=1/(g->m.dz[i+1]/2/g->m.cond[i+1]/g->m.dxy/g->m.dxy + g->m.dz[i]/2/g->m.cond[i]/g->m.dxy/g->m.dxy);
		g->powerfactor[i]=(4*Gx+Gz1+Gz2);//反馈的分母;
		g->facfactor=(3*Gx+Gz1+Gz2);
		g->angfactor=(2*Gx+Gz1+Gz2);
	}
	g->maxxx=xmax;
	g->maxyy=ymax;
	//输出model结构（用于验证）
	/*FILE *fp=fopen("model1_1.txt","w");
		for(int i=0;i<g->m.laynum;i++)
		{
			fprintf(fp,"(%d %d)\(%d %d) z %d\n",g->minx[i],g->maxx[i],g->miny[i],g->maxy[i],g->zval[i]);
		}
	fclose(fp);*/
}

void readpowermap(grid* g,FILE* f)
{
	char line[GRID_BUFFERSIZE];
	int begin=0;
	int end=0;

	while(fgets(line,GRID_BUFFERSIZE,f))
	{
		if (begin)
		{
			if (strcmp(line,"*END\n")==0)
			{
				end=1;
				break;
			}
			else if (strcmp(line,"\n")==0)
			{
				continue;
			}
			else if (line[0]=='/' && line[1]=='/')
			{
				continue;
			}
			else if (line[0]=='*')
			{
				char cmd[GRID_BUFFERSIZE];
				sscanf(line,"%s",cmd);
				if (strcmp(cmd,"*source")==0)
				{
					double tx1,tx2,ty1,ty2,tz,power;
					sscanf(line,"*source %lf %lf %lf %lf %lf %lf",&tx1,&tx2,&ty1,&ty2,&tz,&power);

					double x1,x2,y1,y2;
					int z;
					int player=0;

					for (int i=0; i<g->m.laynum; i++)
					{
						tz-=(zvalue(i)-zvalue(i-1) )*g->m.dz[i];
						if (tz<-EPSILON)
						{
							player=i;//第几层芯片
							tz+=(zvalue(i)-zvalue(i-1))*g->m.dz[i];
							break;
						}
					}

					x1=tx1/(g->m.dxy)+g->minx[player];
					x2=tx2/(g->m.dxy)+g->minx[player];
					y1=ty1/(g->m.dxy)+g->miny[player];
					y2=ty2/(g->m.dxy)+g->miny[player];
					z=zvalue(player-1)+tz/(g->m.dz[player])+ROUND;
					power*=((tx2-tx1)*(ty2-ty1));
					power/=((x2-x1)*(y2-y1));

					if (g->power[z]==NULL)
					{
						g->power[z]=new double* [g->maxx[player]];
						for (int j=g->minx[player]; j<g->maxx[player]; j++)
						{
							g->power[z][j]=new double [g->maxy[player]];
							for (int k=g->miny[player]; k<g->maxy[player]; k++)
								g->power[z][j][k]=0;
							
						}
					}

					for (int j=x1; j<x2; j++)
					{
						for (int k=y1; k<y2; k++)
						{
							g->power[z][j][k]+=power*( minval((double)j+1.0,x2)-maxval((double)j,x1) )* \
								( minval((double)k+1.0,y2)-maxval((double)k,y1) );
						}
					}
				}
			}		
		}
		else
		{
			if (strcmp(line,"BEGIN\n")==0)
			{
				begin=1;
			}
		}
	}
	if (!end)
	{
		fprintf(stderr,"No token '*END\\n' after 'BEGIN' in powermap file.\n");
		exit(146);
	}
}
void readpowermap(grid* g,char* filename)
{
	FILE* file;
	if ( (file=fopen(filename,"r")) )
	{
		readpowermap(g,file);
		fclose(file);
	}
	else
	{
		fprintf(stderr,"Invalid powermap file:'%s'.\n",filename);
		exit(145);
	}
}

void freemem(model *m)//释放内存;
{
	delete[] m->x1;
	delete[] m->x2;
	delete[] m->y1;
	delete[] m->y2;
	delete[] m->zval;
	delete[] m->cond;
	delete[] m->dz;
		
	return;
}
void freemem(grid *g)//释放内存;
{
	for(int i=0;i<g->m.laynum;i++)
	{
		for(int j=0;j<3;j++)
		{
			delete[] g->p[i][j];
			delete[] g->p1[i][j];
			delete[] g->p2[i][j];
		}
		int temp=i-1>=0?g->minx[i-1]:g->minx[0];
		if(g->minx[i]!=temp)//内下特;
		{
			delete[] g->p[i][3];
		}
		delete[] g->p[i];
		delete[] g->p1[i];
		delete[] g->p2[i];
	}
	delete[] g->p;
	delete[] g->p1;
	delete[] g->p2;

	for(int i=0;i<g->maxz;i++)
	{
		if(g->power[i]!=NULL)
		{
			int ln;//laynum;
			for(ln=0;ln<g->m.laynum;ln++)//确定第i层体积元属于哪层芯片;
			{
				if(g->zval[ln]>i)
					break;
			}
			for(int j=g->minx[ln];j<g->maxx[ln];j++)
				delete[] g->power[i][j];
			delete[] g->power[i]; 
		}		
	}
	delete[] g->power;

	delete[] g->minx;
	delete[] g->miny;
	delete[] g->maxx;
	delete[] g->maxy;
	delete[] g->zval;
	delete[] g->prop;
	delete[] g->powerfactor;
		
	return;	
}

