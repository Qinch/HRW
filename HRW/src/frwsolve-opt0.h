#ifndef FRWSOLVER_OPT0_H_	
#define FRWSOLVER_OPT0_H_

#include <malloc.h>
#include <cstdlib>
#include<cstdarg>
#include "random.h"
#include"macrostruct.h"
#include"grid.h"

#define gminx(i) (i>=0? g->minx[i]:g->minx[0])
#define gmaxx(i) (i>=0? g->maxx[i]:g->maxx[0])
#define gminy(i) (i>=0? g->miny[i]:g->miny[0])
#define gmaxy(i) (i>=0? g->maxy[i]:g->maxy[0])
#define gzval(i) (i>=0? g->zval[i]:0)
#define gmdz(i) (i>=0? g->m.dz[i]:0)
#define gmzval(i) (i>=0? g->m.zval[i]:0)
#define LONGWALK 29999999
#define EPSX 1e-6

#define X 0
#define Y 1
#define Z 2
#define TESTRGN(x,y,lay,width) ( (x>(gminx(lay)+width))&&(x<(gmaxx(lay)-width)) \
										&&(y>(gminy(lay)+width))&&(y<(gmaxy(lay)-width)) )
#define TESTINTFACE(bits) ((bits&0x01)==0x01) //�ж��Ƿ�λ������оƬ������;
#define TESTFACZ(bits) (((bits>>1)&0x01)==0x01)//facz=1��ʾ<0��facz=0��ʾ>=0;
#define TESTSPEC(bits,sign) ( ((bits>>(0x02+sign))&0x01)==0x01) //�ж��Ƿ���ܻ��ԽоƬ������,sign=0��ʾ-����,sign=1��ʾ������;
#define TESTPN(bits,type,size,sign) (\
						(((bits>>(type+4))&0x01)==0x1)? \
						(((bits>>(type*4+7+sign*size))&0x3)==0x3):0 \
								 )//sign=0��ʾ������sign=1��ʾ������,�����ж��Ƿ񵽴������߸���������ֵ(Z,size=4;XY,ratio=2); 
#define TESTZHALF(bits,sign) ( \
						(((bits>>(Z*4+9+sign*4))&0x1)==0x1) \
								)//�Ƿ�+-0.5,sign=0��ʾ�Ƿ�+0.5��sign=1��ʾ�Ƿ�-0.5;
#define TESTZLAY(bits,sign) (\
								(((bits>>(Z*4+10+sign*4))&0x1)==0x1) \
							)//lay�Ƿ�+-1,sign=0��ʾ�Ƿ�-1��sign=1��ʾ�Ƿ�+1;
#define TESTSETNZLAY(bits,sign) (\
								(((bits>>(Z*4+10+sign*4))&0x00)) \
							)//lay�Ƿ�+-1,sign=0��ʾ�Ƿ�-1��sign=1��ʾ�Ƿ�+1;
#define TESTSCALE(bits,type,size,sign) ( \
						(((bits>>(type+4))&0x01)==0x1)? \
								(((bits>>(type*4+7+sign*size))&0x01)==0x1):0 \
								 )//sign=0��ʾ������sign=1��ʾ������,�����ж��Ƿ񵽴������߸���������ֵ(Z,size=4;XY,ratio=2),������fac[x,y,z]λʱ�õ�; 

#define TESTINTERSECT(bits) (\
								((bits>>23)&0x01)==0x01 \
								 )//�ж��Ƿ񵽴�����оƬ������;

double frwsolve_opt0(grid* g,macro *mac,int *med,double z,int initlayer,double error,
						double& hops,int &walks,double sentryP,double sentryW,double sentryH,double sentryC,bool args_a,...);

//��x1,x2��ѡȡ��С��ֵ;
inline void getscale2(double x1,double x2,unsigned int bit1,unsigned int bit2,double &scale,unsigned int &bits)
{
	if(x1<x2)
	{
		bits|=bit1;
		scale=x1;
	}
	else if(x1>x2)
	{
		bits|=bit2;
		scale=x2;
	}
	else//x1==x2;
	{
		scale=x1;
		bits|=(bit1|bit2);
	}
}

//��x1,x2,x3��ѡȡ��С��ֵ;
inline void getscale3(double x1,double x2,double x3,unsigned int bit1,unsigned int bit2,unsigned int bit3,double &scale,unsigned int &bits)
{
	unsigned int tbit=0;//temp bits;
	double tx;//temp x;
	getscale2(x1,x2,bit1,bit2,tx,tbit);
	getscale2(tx,x3,tbit,bit3,scale,bits);
}

inline bool equal(double i,int j)
{
	if(fabs(i-(double)j)<EPSX)
		return true;
	else
		return false;
}

inline int getround(double r)
{
	int temp=(int)r;
	if(equal(r,temp+1))
		return temp+1;
	else
		return temp;

}
inline double getfactor(grid *g,int x,int y,int pln)//pln:power laynum,����pln��оƬ���ú������ڻ�ȡÿ��������ߵķ����ķ�ĸ;
{
	double factor;
	if( (x==g->minx[pln]) || (x==g->maxx[pln]-1) )
	{
		if( (y==g->miny[pln]) || (y==g->maxy[pln]-1) )
			factor=g->angfactor;
		else
			factor=g->facfactor;				
	}
	else if( (y==g->miny[pln]) || (y==g->maxy[pln]-1) )
		factor=g->facfactor;
	else
		factor=g->powerfactor[pln];
	return factor;
}

inline void round(int t,int &ret,double prob,int flag,Random& rand)
{
	if(rand.getrand()<prob)
		ret=t+flag;
	else
		ret=t;
	return;
}

//��xy�����������;
inline void cxy2xy(grid* g,double tx,double ty,int lay,int &x,int &y,Random &rand)
{
		double prob;
		if((tx<(g->minx[lay]+0.5))||(tx>(g->maxx[lay]-0.5)) )//�������Ҳ���;
		{
			if(tx<g->minx[lay]+0.5)
				x=g->minx[lay];
			else
				x=g->maxx[lay]-1;
			if(ty<(g->miny[lay]+0.5))//λ���ĸ���;
				y=g->miny[lay];
			else if(ty>(g->maxy[lay]-0.5))
				y=g->maxy[lay]-1;
			else//λ���������Ҳ���;
			{
				int tempy=(int)ty;
				if( ty< (tempy+0.5) )
				{
					prob=0.5-(ty-tempy);
					round(tempy,y,prob,-1,rand);
				}
				else
				{
					prob=(ty-tempy-0.5);
					round(ty,y,prob,1,rand);
				}
			}
		}
		else if( (ty<(g->miny[lay]+0.5))||(ty>(g->maxy[lay]-0.5)) )//ǰ��ͺ���;
		{
			if(ty<g->miny[lay]+0.5)
				y=g->miny[lay];
			else
				y=g->maxy[lay]-1;
			
			int tempx=(int)tx;
			if( tx< (tempx+0.5) )
			{
				prob=0.5-(tx-tempx);
				round(tempx,x,prob,-1,rand);
			}
			else
			{
				prob=(tx-tempx-0.5);
				round(tempx,x,prob,1,rand);
			}
		}
		else//�ڲ�;
		{
				int tempx=(int)tx;
				int tempy=(int)ty;
				if( tx< (tempx+0.5) )
				{
					prob=0.5-(tx-tempx);
					round(tempx,x,prob,-1,rand);
				}
				else
				{
					prob=(tx-tempx-0.5);
					round(tempx,x,prob,1,rand);
				}
		
				if( ty<(tempy+0.5) )
				{
					prob=0.5-(ty-tempy);
					round(tempy,y,prob,-1,rand);
				}
				else
				{
					prob=(ty-tempy-0.5);
					round(tempy,y,prob,1,rand);
				}
		}
		return ;
}


//��z�����������;
inline void cz2z(grid *g,double tz,int x,int y,int &lay,int &z,Random& rand)
{
		double prob;
		if(tz<0.5)
			z=0;
		else if(tz<gzval(lay-1)+0.5)
		{
			if( (x>gminx(lay-1))&&(x<gmaxx(lay-1))&&(y>gminy(lay-1))&&(y<gmaxy(lay-1)) )
			{
				prob=gmdz(lay)*(gzval(lay-1)+0.5-tz)/(gmdz(lay)+gmdz(lay-1));//��1�ĸ���;
				round(gzval(lay-1),z,prob,-1,rand);
				if(z<gzval(lay-1))
					lay--;
			}
			else
				z=gzval(lay-1);
		}
		else if((tz>(g->maxz-0.5)) )
			z=g->maxz-1;
		else if(tz>gzval(lay)-0.5)
		{
			prob=gmdz(lay)*(tz-gzval(lay)+0.5)/(gmdz(lay)+gmdz(lay+1));//��1�ĸ���;
			round(gzval(lay)-1,z,prob,+1,rand);
			if(z>gzval(lay)-1)
				lay++;
		}
		else
		{
				int tempz=(int)tz;
				if( tz<(tempz+0.5) )
				{
					prob=0.5-(tz-tempz);
					round(tempz,z,prob,-1,rand);
				}
				else
				{
					prob=(tz-tempz-0.5);
					round(tempz,z,prob,1,rand);
				}
		}
		return ;
}

//��xyz�����������;
inline void cxyz2xyz(grid* g,double tx,double ty,double tz,int &lay,int &x,int &y,int &z,Random &rand)//mz��ʾ�Ƿ��z�����������;
{
	cxy2xy(g,tx,ty,lay,x,y,rand);
	cz2z(g,tz,x,y,lay,z,rand);
	return ;
}

#endif
