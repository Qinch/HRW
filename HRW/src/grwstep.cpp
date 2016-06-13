#include <cstdio>
#include "grwstep.h"

#define gzval(n) ( n>=0? g->zval[n]:0 )

inline bool changePos(unsigned short sh,grid *g,int &x,int &y,int &z,int &layer,Random &rand)//对位表进行解析，并执行相应的xyz行走;
{
		double *prob=NULL;
		unsigned char parse;
		parse=sh&0x03;
		sh=sh>>2;
		if(parse==0)//内;
				prob=g->p[layer][sh&0x03];
		else if(parse==1)//面;
				prob=g->p2[layer][sh&0x03];
		else if(parse==2)//角;
				prob=g->p1[layer][sh&0x03];
		else//解析失败;
				return false;
		sh=sh>>2;

		//进行xyz方向的解析;
		double rnd=rand.getrand();
		for(int i=0; i<2; i++) //x方向;
		{
				if( (sh&0x01)==1 )
				{
						if( (rnd-=prob[0])<0 )
						{
								if(i==0)
										x--;
								else
										x++;
								return true;
						}
				}
				sh=sh>>1;
		}
		for(int i=0; i<2; i++) //y方向;
		{
				if( (sh&0x01)==1)
				{
						if( (rnd-=prob[0])<0 )
						{
								if(i==0)
										y--;
								else
										y++;
								return true;
						}
				}
				sh=sh>>1;
		}

		//z方向;
		int temp=sh&0x01;//如果temp==1,则z++优先;如果temp==0,则z--优先;
		sh=sh>>1;

		for(int i=0; i<2; i++)
		{
				if( (sh&0x01)==1 )//是否进行z方向的行走;
				{
						if(i==0)
						{
								if( (rnd-=prob[1])<0 )
								{
										if(temp==1)
										{
												z++;
												if( (sh&0x02)==2 )
														layer++;
										}
										else
										{
												z--;
												if( (sh&0x02)==2 )
														layer--;
										}
										return true;
								}
						}
						else //i==1;
						{
								if(temp==1)
								{
										z--;
										if( (sh&0x02)==2)
												layer--;
								}
								else
								{
										z++;
										if( (sh&0x02)==2)
												layer++;
								}
								return true;
						}

				}
				sh=sh>>2;
		}
		return false;
}

inline unsigned short setBits(unsigned short st[],int n)//对位表的每一个字段设值，其中st的下标为0~5;
{
		unsigned short temp=0;
		for(int i=n;i>=0;i--)
		{
				if(i==4)
				{
						temp=(temp<<3)+st[i];
				}
				else
						temp=(temp<<2)+st[i];
		}
		return temp;
}

void grwstep(grid* g,int& x,int& y,int& z,int& layer,Random &rand)
{
				unsigned short sh=0;
				unsigned short st[6];
				if( (x==g->minx[layer]) || (x==g->maxx[layer]-1) )//左侧面或者右侧面;
				{
							if(x==g->minx[layer])//左侧面;
										st[2]=0x02;//10 x++;
							if(x==g->maxx[layer]-1)//右侧面;
										st[2]=0x01;//01 x--;
							if(y==g->miny[layer])//角;
							{
										st[0]=0x02;//10
										st[3]=0x02;//10 y++;
							}
							else if(y==g->maxy[layer]-1)//角;
							{
										st[0]=0x02;//10;
										st[3]=0x01;//01 y--;
							}
							else//面;
							{
										st[0]=0x01;//01;
										st[3]=0x03;//11 y++ y--;
							}
				}
				else if(y==g->miny[layer])//前面;
				{
							st[0]=0x01;//01;
							st[2]=0x03;//11 x++ x--;
							st[3]=0x02;//10 y++;
				}
				else if(y==g->maxy[layer]-1)//后面;
				{
							st[0]=0x01;//01;
							st[2]=0x03;//11 x++ x--;
							st[3]=0x01;//01 y--;
				}
				else//芯片内部;
				{
							st[0]=0x00;//00内;
							st[2]=0x03;//11 x++ x--;
							st[3]=0x03;//11 y++ y--;
				}
		
				//判断上中下;
				if( (z>gzval(layer-1)) && (z<gzval(layer)-1))//中,power层芯片不可能满足该条件;
				{	
							st[1]=0x01;//01 中;
							st[4]=0x03;//011 z++；
							st[5]=0x01;//01 z--;
				}
				else if( z==gzval(layer-1) )//下;
				{
							st[1]=0x00;//00 下;
							//第0层体积元或者两层芯片的非接触界面;
							if( (z==0) || ( (x<g->minx[layer-1]) || (x>=g->maxx[layer-1]) 
									|| (y<g->miny[layer-1]) || (y>=g->maxy[layer-1]) ) )//短路原理;
							{
									st[4]=0x03;//011 z++;
									st[5]=0x00;//00 ;
									if( (z!=0)&&(x!=g->minx[layer]) && (x!=g->maxx[layer]-1) 
										&& (y!=g->miny[layer]) && (y!=g->maxy[layer]-1) )
									{
											st[1]=0x03;//11 对应内特;
									}
							}
							else//两层芯片的接触面;
							{
										int interval=gzval(layer)-gzval(layer-1);
										if(interval==1)
										{
												st[4]=0x07;//111 z++ layer++;
												st[5]=0x03;//11  z-- layer--;
										}
										else
										{
												st[4]=0x03;//011 z++;
												st[5]=0x03;//11 z-- layer--;
										}
							}
				}
				else if( (z==g->maxz-1) || (z==gzval(layer)-1) )//上;
				{
							st[1]=0x02;//10 上;
							if(z==g->maxz-1)//只进行z方向上的行走;
							{
									st[2]=0x00;//00;
									st[3]=0x0;//00;
									st[4]=0x02;//010 z--;
									st[5]=0x03;//11 z++ layer++;
							}
							else
							{
									st[4]=0x02;//010 z--;
									st[5]=0x03;//11 z++ layer++;
							}
				}
		unsigned short temp=setBits(st,5);
		changePos(temp,g,x,y,z,layer,rand);		
}
