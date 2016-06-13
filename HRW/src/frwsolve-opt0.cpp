#include<cstdio>
#include<cmath>
#include<functional>
#include"frwsolve-opt0.h"
#include"grwstep.h"
#include"random.h"
#include"threadpool.h"

extern pthread_spinlock_t _lock;
bool _end=false;
extern struct threadpool *pool;

struct global_param
{
			double sentryP;//
			double sentryH;//
			double sentryW;//
			double sentryC;//
			double t;//
			double tsquare;//
			double error;//
			double macprop;
			double factor;//
			double walkx;//
			double walky;//
			double walkz;//
			int layer;//
			int macsize;//
			int hops;//
			grid* g;//
			macro* mac;//
			int *med;
			int walks;//
			global_param(double P,double H,double W,double C,double err,double prop,double fac,double x,double y,double z,int lay,int size,grid* gd,macro* mo,int *md):\
				sentryP(P),sentryH(H),sentryW(W),sentryC(C),factor(fac),walkx(x),walky(y),walkz(z),layer(lay),macsize(size),g(gd),mac(mo),error(err),macprop(prop),med(md)
			{
				t=0;
				tsquare=0;
				hops=0;
				walks=0;

			}
};
inline void genjump(grid* g,double& walkx,double& walky,double&walkz,int &lay,double macprop,\
						double facx,double facy,double facz,double scale,unsigned int &bits,double sentryW)//flag=true表示采用的是位于两层芯片交接的转移区域表;
{
	//double temp=macprop*scale/g->prop[lay];
	//walkx+=(temp*facx);//更新x坐标;
	//walky+=(temp*facy);//更新y坐标;
	int templay=lay;
	if(TESTPN(bits,Z,4,1))//是否到达与上层芯片的交界面;
	{
		int tlay;
		if(TESTSPEC(bits,1))//判断当前层是否为第6层芯片;
			tlay=lay+1;
		else
			tlay=lay;
		if(TESTZHALF(bits,1))//是否-0.5;
			walkz=gzval(tlay)-0.5;
		else
		{
			walkz=gzval(tlay);
			bits|=(1<<23);//表示到达两层芯片交界面;//////////////////qinchao
		}
		if(TESTZLAY(bits,1))//判断是否lay++;
			lay++;
	}
	else if(TESTPN(bits,Z,4,0))//到达与下层芯片的交界面;
	{
		int tlay;
		if( (TESTINTFACE(bits))||TESTSPEC(bits,0) )//是否两层芯片交界面,或者是否是第7层芯片;
			tlay=lay-2;
		else 
			tlay=lay-1;
		if(TESTZHALF(bits,0))//是否+0.5;
			walkz=gzval(tlay)+0.5;
		else
		{
			if(TESTRGN(walkx,walky,tlay,0))//不用缩小zscale的区域;
			{
				walkz=gzval(tlay);
			//	if(TESTRGN(walkx,walky,tlay,sentryW))
				bits|=(1<<23);//表示到达两层芯片交界面;
			}
			else//缩小zscale;
			{
				if(TESTINTFACE(bits)&&TESTFACZ(bits))//两层芯片交界面并且facz<0;
				{
					scale=(walkz-(gzval(tlay)+0.5))*gmzval(lay-1)/gmzval(lay)*250.5/250;//qinchao;
				}
				else
					scale=(walkz-(gzval(tlay)+0.5))*250.5/250;
				bits=bits&(~(0x030));//将XY位置0;
			//	TESTSETNZLAY(bits,0);//关闭lay--;
				walkz=gzval(tlay)+0.5;
			}
		}
		if(TESTZLAY(bits,0))//判断是否lay--;
			lay--;
	}
	else//更新z;
	{	
		double temp=scale*facz;
		if(TESTINTFACE(bits))//在两层芯片交界面;
		{
			if((!TESTFACZ(bits))&&(facz<0))
			{
				printf("error %d\n",TESTFACZ(bits));
				exit(0);
			}
			if(TESTFACZ(bits))//facz<0;
			{
				walkz+=temp*gmdz(lay)/gmdz(lay-1);
				lay--;
			}
			else
				walkz+=temp;
		}
		else//不在两层芯片交界面;
		{
			walkz+=temp;
			if(TESTSPEC(bits,0)&&(TESTFACZ(bits)))//是否第7层芯片,并且facz<0;&&TESTFACZ(bits)
			{
				if(walkz<gzval(lay-1))
				{
					lay--;
				}
			}
			else if(TESTSPEC(bits,1)&&(!TESTFACZ(bits)))//是否第6层芯片,并且facz>=0;&&(!TESTFACZ(bits))
			{
				if(!(walkz<gzval(lay)))//((walkz>gzval(lay))||fabs(walkz-gzval(lay))<1e-6 )
				{
					lay++;
				}
			}
		}
	}
	
	double temp=macprop*scale/g->prop[templay];
	if(TESTINTFACE(bits))//两层芯片交界面;
		templay--;
	if(TESTPN(bits,X,2,0))//到达X负方向的最大值;
		walkx=gminx(templay)+0.5;
	else if(TESTPN(bits,X,2,1))//到达X正方向的最大值;
		walkx=gmaxx(templay)-0.5;
	else
		walkx+=(temp*facx);//更新x坐标;
	if(TESTPN(bits,Y,2,0))//到达Y负方向的最大值;
		walky=gminy(templay)+0.5;
	else if(TESTPN(bits,Y,2,1))//到达Y正方向的最大值;
		walky=gmaxy(templay)-0.5;
	else
		walky+=(temp*facy);//更新y坐标;
	
	return ;
}

void walkbound(grid* g,macro* mac,int *med,double macprop,int macsize,\
					double& x,double& y,double& z,int& lay ,bool &pref2g,double sentryW,unsigned int &bits,Random &rand)//sentryW为红色区域的宽度;
{
	double half=0.5;//微调;
	bits=0;
	//if( (x>(gminx(lay-1)+sentryW))&&(x<(gmaxx(lay-1)-sentryW))&&(y>(gminy(lay-1)+sentryW))&&(y<(gmaxy(lay-1)-sentryW)))
	if(TESTRGN(x,y,lay-1,sentryW))
	{
			bits|=0x01;//两层芯片的交界面置位;
			double x1=x-(gminx(lay-1)+half);//x-
			double x2=(gmaxx(lay-1)-half)-x;//x+
			double xscale;
			unsigned int bx=0;
			double ratio=g->prop[lay]/macprop;
			getscale2(x1,x2,0x090,0x0210,xscale,bx);
			xscale=xscale*ratio;//换算成交界面上方的z方向的体积元的个数;
			double y1= y-(gminy(lay-1)+half); //y-;
			double y2=(gmaxy(lay-1)-half)-y; //y+;
			double yscale;
			unsigned int by=0;
			getscale2(y1,y2,0x0820,0x02020,yscale,by);
			yscale=yscale*ratio;//换算成交界面上方的z方向的体积元的个数;
			double z1;
			double z2;
			double zscale;
			unsigned int bz=0;
			if(lay==6)
			{
				z1=(z-gzval(lay-2))*gmdz(lay-1)/gmdz(lay);
				z2=(gzval(lay+1)-0.5)-z;
				getscale2(z1,z2,0x048040,0x0680040,zscale,bz);
				bits|=0x08;
			}
			else if(lay==3)
			{
				z1=(z-(gzval(lay-2)+half))*gmdz(lay-1)/gmdz(lay);
				z2=gzval(lay)-z;
				getscale2(z1,z2,0x068040,0x0480040,zscale,bz);
				
			}
			else
			{
				z1=(z-gzval(lay-2))*gmdz(lay-1)/gmdz(lay);
				z2=gzval(lay)-z;
				getscale2(z1,z2,0x048040,0x0480040,zscale,bz);
			}
			vector<double> *tvec;//tempvec;
			tvec=mac->intersect[med[lay-1]][med[lay]];
			double finalscale;
			getscale3(xscale,yscale,zscale,bx,by,bz,finalscale,bits);
			finalscale=finalscale*250.5/250;
			int idx=rand.getrand()*macsize;//跳转区域表的索引号;
			double facx=tvec[0][idx];
			if( TESTSCALE(bits,X,2,0)&&(fabs(facx+250/250.5)<1e-4) )//x-;
				bits=bits|0x0100;
			else if( TESTSCALE(bits,X,2,1)&&(fabs(facx-250/250.5)<1e-4) )//x+;
				bits=bits|0x0400;
			double facy=tvec[1][idx];
			if( TESTSCALE(bits,Y,2,0)&&(fabs(facy+250/250.5)<1e-4))
				bits=bits|0x01000;
			else if( TESTSCALE(bits,Y,2,1)&&(fabs(facy-250/250.5)<1e-4) )
				bits=bits|0x04000;
			double facz=tvec[2][idx];
			if(facz<0)
			{
				if(TESTSCALE(bits,Z,4,0)&&(fabs(facz+250/250.5)<1e-4))
					bits=bits|0x010000;
				bits=bits|0x02;//该位为1表示非小于0;
			}
			else
			{
				if(TESTSCALE(bits,Z,4,1)&&(fabs(facz-250/250.5)<1e-4))
					bits=bits|0x0100000;
				
			}
			genjump(g,x,y,z,lay,macprop,facx,facy,facz,finalscale,bits,sentryW);
			pref2g=true;//表示该次行走为FRW;
	}
	else//进行GRW行走;
	{
			int tempx;
			int tempy;
			int tempz;
			if(pref2g)//之前一次hop为frw,所以现在进行XYZ坐标修正;
			{
					cxyz2xyz(g,x,y,z,lay,tempx,tempy,tempz,rand);
			}
			else
			{
				tempx=(int)x;
				tempy=(int)y;
				tempz=(int)z;
			}
			grwstep(g,tempx,tempy,tempz,lay,rand);
			x=tempx+0.5;
			y=tempy+0.5;
			z=tempz+0.5;
			pref2g=false;
	}
	return ;
}

//某层芯片或者某几层热导率相同的芯片的内部;
//sentryP为蓝色区域的高度;
//sentryW为红色区域的宽度;
//sentryH为红色区域的高度;
//sentryC为黄色区域的高度;
void walkinner( grid* g,macro* mac,double macprop,int macsize,double& x,double& y,double& z,int &lay,bool &pref2g,\
						double &patht,double sentryP,double sentryW,double sentryH,double sentryC,unsigned int &bits,Random &rand)//lay为当芯片的层数(从0开始计数);
{
	bits=0;
		bool boolz,boolxy;
		double zscale;
		double xscale;
		double yscale;
		double half=0.5;
		unsigned int bz1=0;
		unsigned int bz2=0;
		unsigned int bz=0;
		unsigned int bx=0;
		unsigned int by=0;
		double z1,z2;
		double x1,x2;
		double y1,y2;
	/*	if( //两层芯片长宽不相等时，位于两岑芯片的非接触面的情况;
			(gminx(lay-1)!=gminx(lay))&& \
						( \
							(x<(gminx(lay-1)+sentryW))||(x>(gmaxx(lay-1)-sentryW))||(y<(gminy(lay-1)+sentryW))||(y>(gmaxy(lay-1)-sentryW))|| \
								equal(x,gminx(lay-1)+sentryW)||equal(x,gmaxx(lay-1)-sentryW)||equal(y,gminy(lay-1)+sentryW)||equal(y,gmaxy(lay-1)-sentryW) 
						) \
						&&(z>(gzval(lay-1)+sentryH))  \
			)*/
		if( (gminx(lay-1)!=gminx(lay))&&(!TESTRGN(x,y,lay-1,sentryW))&&(z>(gzval(lay-1)+sentryH)) )
		{
				if(lay==6)
				{
					z1=z-gzval(lay-1);//下交界面;
					bz1=0x08040;
					z2=(gzval(lay+1)-0.5)-z;//上交界面;
					bz2=0x0680040;
					bits|=0x08;//设置是否可能跨越上层交界面;
				}
				else
				{
					z1=z-gzval(lay-1);
					bz1=0x08040;
					z2=gzval(lay)-z ;
					bz2=0x0480040;
				}
				boolz=true;
		}
		//else if( (x>(gminx(lay-1)+sentryW))&&(x<(gmaxx(lay-1)-sentryW))&&(y>(gminy(lay-1)+sentryW))&&(y<(gmaxy(lay-1)-sentryW)) )
		else if(lay==6||lay==7)
		{
				if(lay==6)
				{
					z1=z-gzval(lay-1);//下交界面;
					bz1=0x08040;
					z2=(gzval(lay+1)-0.5)-z;//上交界面;
					bz2=0x0680040;
					bits|=0x08;//设置是否可能跨越上层交界面;
					boolz=true;
				}
				else if(lay==7)
				{
					if(z<(gzval(lay)-sentryC))
					{
						z1=z-gzval(lay-2);
						bz1=0x048040;
						z2=(gzval(lay)-0.5)-z;
						bz2=0x0280040;
						bits|=0x04;
						boolz=true;
					}
					else
						boolz=false;
				}
		}
		else if(TESTRGN(x,y,lay-1,sentryW))
		{
				if(lay==0)//位于第0层芯片的警戒区域之外;
				{
					if((z<(gzval(lay)-sentryP)))
					{
						z1=10000;
						bz1=0x028040;
						z2=(gzval(lay)-half)-z;
						bz2=0x0280040;
						bits|=(0x01<<24);
						boolz=true;
					}
					else
						boolz=false;
				}
				else if(lay==2)
				{
					if(z>(gzval(lay-1)+sentryP))
					{
						z1=z-(gzval(lay-1)+half);
						bz1=0x028040;
						z2=gzval(lay)-z;
						bz2=0x0480040;
						boolz=true;
					}
					else
						boolz=false;
				}
				else
				{
				
					z1=z-gzval(lay-1);
					bz1=0x08040;
					z2=gzval(lay)-z;
					bz2=0x0480040;
					boolz=true;
				}
		}
		else
		{
				boolz=false;
		}

		//if((x>(gminx(lay)+sentryW))&&(x<(gmaxx(lay)-sentryW))&&(y>(gminy(lay)+sentryW))&&(y<(gmaxy(lay)-sentryW)) )
		if(lay==6||lay==7)
		{
				x1=x2=y1=y2=1000000;
				boolxy=true;
		}
		else if(TESTRGN(x,y,lay,sentryW))
		{
				x1= x-(gminx(lay)+half);
				x2=(gmaxx(lay)-half)-x ;
				y1= y-(gminy(lay)+half);
				y2=(gmaxy(lay)-half)-y;
				boolxy=true;
		}
		else
			boolxy=false;

		if( (lay!=1)&&boolz&&boolxy )
		{
				double finalscale;
				if(lay==6||lay==7)
				{
					getscale2(z1,z2,bz1,bz2,finalscale,bits);
				}
				else
				{
					getscale2(x1,x2,0x090,0x0210,xscale,bx);
					xscale=xscale*g->prop[lay]/macprop;
					getscale2(y1,y2,0x0820,0x02020,yscale,by);
					yscale=yscale*g->prop[lay]/macprop;
					getscale2(z1,z2,bz1,bz2,zscale,bz);
					getscale3(xscale,yscale,zscale,bx,by,bz,finalscale,bits);
				}
				finalscale=finalscale*250.5/250;
				int idx=rand.getrand()*macsize;
				double facx=mac->inner[0][idx];
				if( TESTSCALE(bits,X,2,0)&&(fabs(facx+250/250.5)<1e-4) )//x-;
					bits=bits|0x0100;
				else if( TESTSCALE(bits,X,2,1)&&(fabs(facx-250/250.5)<1e-4) )//x+;
					bits=bits|0x0400;
				double facy=mac->inner[1][idx];
				if( TESTSCALE(bits,Y,2,0)&&(fabs(facy+250/250.5)<1e-4))
					bits=bits|0x01000;
				else if( TESTSCALE(bits,Y,2,1)&&(fabs(facy-250/250.5)<1e-4) )
					bits=bits|0x04000;
				double facz=mac->inner[2][idx];
				if(facz<0)
				{
					if(TESTSCALE(bits,Z,4,0)&&(fabs(facz+250/250.5)<1e-4))
						bits=bits|0x010000;
					bits=bits|0x02;//facz位置1,表示facz<0;
				}
				else
				{
					if(TESTSCALE(bits,Z,4,1)&&(fabs(facz-250/250.5)<1e-4))
						bits=bits|0x0100000;
				}
				genjump(g,x,y,z,lay,macprop,facx,facy,facz,finalscale,bits,sentryW);
		if(lay==0)//第0层反射;
		{
			if(z<0)
				z=-z;
		}
		else if(TESTSPEC(bits,0)||TESTSPEC(bits,1))
		{
			if(x<gminx(6))
				x=2*gminx(6)-x;
			else if((x>gmaxx(6)))
				x=2*gmaxx(6)-x;

			if(y<gminy(6))
				y=2*gminy(6)-y;
			else if((y>gmaxy(6)))
				y=2*gmaxy(6)-y;

		}
				pref2g=true;
		}
		else
		{
			int tempx;
			int tempy;
			int tempz;
			if(pref2g)
			{
					cxyz2xyz(g,x,y,z,lay,tempx,tempy,tempz,rand);
			}
			else
			{
				tempx=(int)x;
				tempy=(int)y;
				tempz=(int)z;
			}
			if(lay==1)
			{
				if(g->power[tempz])
					if (g->power[tempz][tempx])
						patht+=(g->power[tempz][tempx][tempy]);
			}
			grwstep(g,tempx,tempy,tempz,lay,rand);
			x=tempx+0.5;
			y=tempy+0.5;
			z=tempz+0.5;
			pref2g=false;
		}
		return ;
}
inline bool get_progress(int walks,double t,double tsquare,double error)//是否结束;
{
	 if( (walks<50) || ((t*t)<walks*(tsquare-t*t*error*error)) )
		 return false;
	 else
		 return true;
}

void* walk(void *args)
{
		// random object
		Random rand;	
		global_param* gp=(global_param*)args;
		double factor=gp->factor;
		grid *g=gp->g;
		macro *mac=gp->mac;
		double macprop=gp->macprop;
		int macsize=gp->macsize;
		double sentryP=gp->sentryP;
		double sentryW=gp->sentryW;
		double sentryH=gp->sentryH;
		double sentryC=gp->sentryC;
		int *med=gp->med;


		int cnt=0;
		double tt=0;
		double ts=0;
		int th=0;
		int step=0;
		double patht=0;
		while(!_end)
		{
			bool pref2g=false;//如果上次行走为frw，则pref2g=true;如果上次行走为grw则pref2g=false;
			unsigned int bits=0;
			patht=0;
			step=0;
			double walkx=gp->walkx;
			double walky=gp->walky;
			double walkz=gp->walkz;
			int layer=gp->layer;

			while(walkz<g->maxz)
			{
				if(TESTINTERSECT(bits))
					walkbound(g,mac,med,macprop,macsize,walkx,walky,walkz,layer,pref2g,sentryW,bits,rand);//只进行两层芯片间的转移区域;
				else//芯片内部;
					walkinner(g,mac,macprop,macsize,walkx,walky,walkz,layer,pref2g,patht,sentryP,sentryW,sentryH,sentryC,bits,rand);
				step++;
			}

			cnt++;//walk的次数;
			if (step>=LONGWALK) 
				continue;
			th+=step;//temp hops;
			patht=patht/factor;	
			patht+=g->m.tout;
			tt+=patht;
			ts+=patht*patht;
	
			
			
			if ( cnt < 10 ) 
				continue;
			else if (pthread_spin_trylock(&_lock) ) 
				continue;
			gp->walks+=cnt;
			gp->hops+=th;
			gp->t+=tt;
			gp->tsquare+=ts;

			cnt=0;
			tt=0;
			ts=0;
			th=0;
	
		
			if(get_progress(gp->walks,gp->t,gp->tsquare,gp->error))//是否结束;
			{
				_end=true;
			}

			pthread_spin_unlock(&_lock);
		}
		pthread_spin_lock(&_lock);
			gp->walks+=cnt;
			gp->hops+=th;
			gp->t+=tt;
			gp->tsquare+=ts;
		pthread_spin_unlock(&_lock);
}

double frwsolve_opt0(grid* g,macro *mac,int *med,double z,int initlayer,double error,
						double& hops,int &walks,double sentryP,double sentryW,double sentryH,double sentryC,bool args_a,...)
{
	//随机行走终止条件；
	//sqrt((sum(t^2)-sum(t)^2/n)/n)/sqrt(n)<err  
	//	sum(t^2)-sum(t)^2/n<n^2*err^2
	//	sum(t)^2/n>sum(t^2)-n^2*err^2
	//  To:sum(t)^2>n(sum(t^2)-n^2*err^2)
	//err=error*sum(t)/n
	//Thus, sum(t)^2>n(sum(t^2)-error^2*t^2)

	walks=0;
	hops=0;
	double factor;
	double t=0;
	double tsquare=0;
	double x,y;
	double tx,ty;
	va_list args;
	va_start(args,args_a);
	if(args_a)//随机生成待测点，自动计算;
	{
		x=va_arg(args,double);
		y=va_arg(args,double);
		factor=va_arg(args,double);
	}
	else//手工输入待测点xy坐标;
	{
		tx=va_arg(args,double);
		ty=va_arg(args,double);
	}
	va_end(args);
	double macprop=mac->ratio*mac->size[0]/mac->size[2];//转移区域的宽高比x/z;
	int macsize=mac->inner->size();

	/*	if(!args_a)//将手工输入的xy坐标点转换为体积元个数计数;
		{
			int tempx,tempy;
			cxy2xy(g,tx,ty,initlayer,tempx,tempy);
			x=(double)tempx;
			y=(double)tempy;
			factor=getfactor(g,tempx,tempy,initlayer);
		}*/
		global_param param(sentryP,sentryH,sentryW,sentryC,error,macprop,factor,x,y,z,initlayer,macsize,g,mac,med);
		_end=false;
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_add_job(pool,walk,(void*)&param);
		threadpool_wait(pool);

		hops=param.hops/param.walks;
		walks=param.walks;
		return param.t/param.walks;
}

