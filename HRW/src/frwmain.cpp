#include<cstring>
#include "argsparase.h"
#include "model.h"
#include "grid.h"
#include "frwsolve-opt0.h"
#include"macrostruct.h"
#include"random.h"
#include"threadpool.h"
struct region
{
	double P;
	double H;
	double W;
	double C;
};
region reg[3]={{2.5,2.5,2.5,2.5},{2.5,2.5,2.5,2.5},{2.5,2.5,2.5,2.5}};
struct threadpool *pool =NULL;
pthread_spinlock_t _lock;

int main(int argc,char** argv)
{
	struct grid g;
	args_parase(argc,argv);//参数解析;
	read(&(g.m),args_model);//读取model文件;
	buildgrid(&g);
	readpowermap(&g,args_powermap);//读取powermap文件;
	macro mac;
	FILE *fmac=fopen(args_table,"rb");
	if(fmac==NULL)
	{
		fprintf(stderr,"Invalid macro file '%s'.\n",args_table);
		exit(145);
	}
	readmac(mac,fmac);
	fclose(fmac);
	int med[8];//med[i]表示第i层芯片对应的热导率在mac.mediums数组中的下标;
	int laysize=g.m.laynum;//芯片的层数;
	int medsize=mac.mediums.size();//芯片材料的种数(根据热导率划分);
	for(int i=0;i<laysize;i++)
	{
		for(int j=0;j<medsize;j++)
			if(fabs(g.m.cond[i]-mac.mediums[j])<1e-6)	
				med[i]=j;
	}
	int z=0;
	for (int i=0; i<g.maxz; i++)
		if(g.power[i])
		{
			z=i;//第几层体积元;
			break;
		}
	int pln=0;
	for (int i=0; i<laysize; i++)
	{
		//powerplayer层体积元属于第几层芯片;
		if (g.zval[i]>z)
		{
			pln=i;
			break;
		}
	}
	int x,y;
	int walks;
	double top=0;
	double temp,hops;
	int tp;
	double factor;//待测点位于power层的哪个位置，facotor=g.powerfacotr[1],表示内部，\
						factor=g.facfactor表示位于侧面（非两个侧面的夹角）,scale=g.angfactor表示位于两个侧面的夹角；
	double sentryP;
	double sentryH;
	double sentryW;
	double sentryC;
	if(args_model[strlen(args_model)-4]=='1')
	{
		sentryP=reg[0].P;
		sentryH=reg[0].H;
		sentryW=reg[0].W;
		sentryC=reg[0].C;
	}
	else if(args_model[strlen(args_model)-4]=='2')
	{
		sentryP=reg[1].P;
		sentryH=reg[1].H;
		sentryW=reg[1].W;
		sentryC=reg[1].C;
	}
	else
	{
		sentryP=reg[2].P;
		sentryH=reg[2].H;
		sentryW=reg[2].W;
		sentryC=reg[2].C;
	}
	if(args_a)//-a,--auto,表示自动计算;
	{
		tp=args_points;//待测点的个数;
	}
	else
		tp=cx.size();
	Random rand;
	pthread_spin_init(&_lock,PTHREAD_PROCESS_PRIVATE);	
	pool = threadpool_init(6,6);
	for (int i=0; i<tp; i++)
	{
		walks=0;
		temp=0;
		hops=0;
		if(args_a)//生成随机计算的体积元;
		{
			x=rand.getrand()*(g.maxx[pln]-g.minx[pln])+g.minx[pln];
			y=rand.getrand()*(g.maxy[pln]-g.miny[pln])+g.miny[pln];
			factor=getfactor(&g,x,y,pln);
			temp=frwsolve_opt0(&g,&mac,med,z+0.5,pln,0.01,hops,walks,sentryP,sentryW,sentryH,sentryC,true,x+0.5,y+0.5,factor);
			top=(temp>top?temp:top);
			printf("(%d,%d) T=%lf,avg.hops=%lf,walks=%d\n",x,y,temp,hops,walks);
		}
		else//计算手工输入的坐标点;
		{
			double tx=0,ty=0;
			tx=cx[i]/g.m.dxy;
			ty=cy[i]/g.m.dxy;
			temp=frwsolve_opt0(&g,&mac,med,z+0.5,pln,0.01,hops,walks,sentryP,sentryW,sentryH,sentryC,false,tx,ty);
			top=(temp>top?temp:top);
			printf("(%lf,%lf) T=%lf,avg.hops=%lf,walks=%d\n",cx[i],cy[i],temp,hops,walks);
		}
	}
	threadpool_destroy(pool);
	pthread_spin_destroy(&_lock);
	printf("Top:%lf\n",top);
	freemem(&g);
	freemem(&mac);
	return 0;
}

