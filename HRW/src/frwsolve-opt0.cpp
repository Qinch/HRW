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
						double facx,double facy,double facz,double scale,unsigned int &bits,double sentryW)//flag=true��ʾ���õ���λ������оƬ���ӵ�ת�������;
{
	//double temp=macprop*scale/g->prop[lay];
	//walkx+=(temp*facx);//����x����;
	//walky+=(temp*facy);//����y����;
	int templay=lay;
	if(TESTPN(bits,Z,4,1))//�Ƿ񵽴����ϲ�оƬ�Ľ�����;
	{
		int tlay;
		if(TESTSPEC(bits,1))//�жϵ�ǰ���Ƿ�Ϊ��6��оƬ;
			tlay=lay+1;
		else
			tlay=lay;
		if(TESTZHALF(bits,1))//�Ƿ�-0.5;
			walkz=gzval(tlay)-0.5;
		else
		{
			walkz=gzval(tlay);
			bits|=(1<<23);//��ʾ��������оƬ������;//////////////////qinchao
		}
		if(TESTZLAY(bits,1))//�ж��Ƿ�lay++;
			lay++;
	}
	else if(TESTPN(bits,Z,4,0))//�������²�оƬ�Ľ�����;
	{
		int tlay;
		if( (TESTINTFACE(bits))||TESTSPEC(bits,0) )//�Ƿ�����оƬ������,�����Ƿ��ǵ�7��оƬ;
			tlay=lay-2;
		else 
			tlay=lay-1;
		if(TESTZHALF(bits,0))//�Ƿ�+0.5;
			walkz=gzval(tlay)+0.5;
		else
		{
			if(TESTRGN(walkx,walky,tlay,0))//������Сzscale������;
			{
				walkz=gzval(tlay);
			//	if(TESTRGN(walkx,walky,tlay,sentryW))
				bits|=(1<<23);//��ʾ��������оƬ������;
			}
			else//��Сzscale;
			{
				if(TESTINTFACE(bits)&&TESTFACZ(bits))//����оƬ�����沢��facz<0;
				{
					scale=(walkz-(gzval(tlay)+0.5))*gmzval(lay-1)/gmzval(lay)*250.5/250;//qinchao;
				}
				else
					scale=(walkz-(gzval(tlay)+0.5))*250.5/250;
				bits=bits&(~(0x030));//��XYλ��0;
			//	TESTSETNZLAY(bits,0);//�ر�lay--;
				walkz=gzval(tlay)+0.5;
			}
		}
		if(TESTZLAY(bits,0))//�ж��Ƿ�lay--;
			lay--;
	}
	else//����z;
	{	
		double temp=scale*facz;
		if(TESTINTFACE(bits))//������оƬ������;
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
		else//��������оƬ������;
		{
			walkz+=temp;
			if(TESTSPEC(bits,0)&&(TESTFACZ(bits)))//�Ƿ��7��оƬ,����facz<0;&&TESTFACZ(bits)
			{
				if(walkz<gzval(lay-1))
				{
					lay--;
				}
			}
			else if(TESTSPEC(bits,1)&&(!TESTFACZ(bits)))//�Ƿ��6��оƬ,����facz>=0;&&(!TESTFACZ(bits))
			{
				if(!(walkz<gzval(lay)))//((walkz>gzval(lay))||fabs(walkz-gzval(lay))<1e-6 )
				{
					lay++;
				}
			}
		}
	}
	
	double temp=macprop*scale/g->prop[templay];
	if(TESTINTFACE(bits))//����оƬ������;
		templay--;
	if(TESTPN(bits,X,2,0))//����X����������ֵ;
		walkx=gminx(templay)+0.5;
	else if(TESTPN(bits,X,2,1))//����X����������ֵ;
		walkx=gmaxx(templay)-0.5;
	else
		walkx+=(temp*facx);//����x����;
	if(TESTPN(bits,Y,2,0))//����Y����������ֵ;
		walky=gminy(templay)+0.5;
	else if(TESTPN(bits,Y,2,1))//����Y����������ֵ;
		walky=gmaxy(templay)-0.5;
	else
		walky+=(temp*facy);//����y����;
	
	return ;
}

void walkbound(grid* g,macro* mac,int *med,double macprop,int macsize,\
					double& x,double& y,double& z,int& lay ,bool &pref2g,double sentryW,unsigned int &bits,Random &rand)//sentryWΪ��ɫ����Ŀ��;
{
	double half=0.5;//΢��;
	bits=0;
	//if( (x>(gminx(lay-1)+sentryW))&&(x<(gmaxx(lay-1)-sentryW))&&(y>(gminy(lay-1)+sentryW))&&(y<(gmaxy(lay-1)-sentryW)))
	if(TESTRGN(x,y,lay-1,sentryW))
	{
			bits|=0x01;//����оƬ�Ľ�������λ;
			double x1=x-(gminx(lay-1)+half);//x-
			double x2=(gmaxx(lay-1)-half)-x;//x+
			double xscale;
			unsigned int bx=0;
			double ratio=g->prop[lay]/macprop;
			getscale2(x1,x2,0x090,0x0210,xscale,bx);
			xscale=xscale*ratio;//����ɽ������Ϸ���z��������Ԫ�ĸ���;
			double y1= y-(gminy(lay-1)+half); //y-;
			double y2=(gmaxy(lay-1)-half)-y; //y+;
			double yscale;
			unsigned int by=0;
			getscale2(y1,y2,0x0820,0x02020,yscale,by);
			yscale=yscale*ratio;//����ɽ������Ϸ���z��������Ԫ�ĸ���;
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
			int idx=rand.getrand()*macsize;//��ת������������;
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
				bits=bits|0x02;//��λΪ1��ʾ��С��0;
			}
			else
			{
				if(TESTSCALE(bits,Z,4,1)&&(fabs(facz-250/250.5)<1e-4))
					bits=bits|0x0100000;
				
			}
			genjump(g,x,y,z,lay,macprop,facx,facy,facz,finalscale,bits,sentryW);
			pref2g=true;//��ʾ�ô�����ΪFRW;
	}
	else//����GRW����;
	{
			int tempx;
			int tempy;
			int tempz;
			if(pref2g)//֮ǰһ��hopΪfrw,�������ڽ���XYZ��������;
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

//ĳ��оƬ����ĳ�����ȵ�����ͬ��оƬ���ڲ�;
//sentryPΪ��ɫ����ĸ߶�;
//sentryWΪ��ɫ����Ŀ��;
//sentryHΪ��ɫ����ĸ߶�;
//sentryCΪ��ɫ����ĸ߶�;
void walkinner( grid* g,macro* mac,double macprop,int macsize,double& x,double& y,double& z,int &lay,bool &pref2g,\
						double &patht,double sentryP,double sentryW,double sentryH,double sentryC,unsigned int &bits,Random &rand)//layΪ��оƬ�Ĳ���(��0��ʼ����);
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
	/*	if( //����оƬ�������ʱ��λ�����оƬ�ķǽӴ�������;
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
					z1=z-gzval(lay-1);//�½�����;
					bz1=0x08040;
					z2=(gzval(lay+1)-0.5)-z;//�Ͻ�����;
					bz2=0x0680040;
					bits|=0x08;//�����Ƿ���ܿ�Խ�ϲ㽻����;
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
					z1=z-gzval(lay-1);//�½�����;
					bz1=0x08040;
					z2=(gzval(lay+1)-0.5)-z;//�Ͻ�����;
					bz2=0x0680040;
					bits|=0x08;//�����Ƿ���ܿ�Խ�ϲ㽻����;
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
				if(lay==0)//λ�ڵ�0��оƬ�ľ�������֮��;
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
					bits=bits|0x02;//faczλ��1,��ʾfacz<0;
				}
				else
				{
					if(TESTSCALE(bits,Z,4,1)&&(fabs(facz-250/250.5)<1e-4))
						bits=bits|0x0100000;
				}
				genjump(g,x,y,z,lay,macprop,facx,facy,facz,finalscale,bits,sentryW);
		if(lay==0)//��0�㷴��;
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
inline bool get_progress(int walks,double t,double tsquare,double error)//�Ƿ����;
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
			bool pref2g=false;//����ϴ�����Ϊfrw����pref2g=true;����ϴ�����Ϊgrw��pref2g=false;
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
					walkbound(g,mac,med,macprop,macsize,walkx,walky,walkz,layer,pref2g,sentryW,bits,rand);//ֻ��������оƬ���ת������;
				else//оƬ�ڲ�;
					walkinner(g,mac,macprop,macsize,walkx,walky,walkz,layer,pref2g,patht,sentryP,sentryW,sentryH,sentryC,bits,rand);
				step++;
			}

			cnt++;//walk�Ĵ���;
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
	
		
			if(get_progress(gp->walks,gp->t,gp->tsquare,gp->error))//�Ƿ����;
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
	//���������ֹ������
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
	if(args_a)//������ɴ���㣬�Զ�����;
	{
		x=va_arg(args,double);
		y=va_arg(args,double);
		factor=va_arg(args,double);
	}
	else//�ֹ���������xy����;
	{
		tx=va_arg(args,double);
		ty=va_arg(args,double);
	}
	va_end(args);
	double macprop=mac->ratio*mac->size[0]/mac->size[2];//ת������Ŀ�߱�x/z;
	int macsize=mac->inner->size();

	/*	if(!args_a)//���ֹ������xy�����ת��Ϊ���Ԫ��������;
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

