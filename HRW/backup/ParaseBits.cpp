#include<cstdio>

//对位表每个字段进行设置；
/*

 *unsigned short st[6];
 *st[0]:大小为2bits,其中分为内(00) 面(01) 角(10);
 *st[1]:大小为2bits,其中分为下(00) 中(01) 上(10)  内下特(11);
 *st[2]:大小为2bits,其中分为x--(01) x++(10)  x++x--(11);
 *st[3]:大小为2bits,其中分为y--(01) y++(10)  y++y--(11);
 *st[4]:大小为3bits(第一个bit为标志位，表示概率数组中对应的存储的是z++还是z--的概率),st[5]大小为2bits:其中分为
	(00 z--010) (z++01 z--010)(z++layer++11 z--010)  
	(00 z-- layer--110) (z++01 z--layer--110) (z++layer++11 z--layer++110)  
	(00 z++011) (z--01 z++011)(z--layer--11 z++011)  
	(00 z++layer++111)(z--01 z++layer++111)(z--layer--11 z++layer++111)

*/
unsigned short setBits(unsigned short st[],int n=5)
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

//对位表每个字段进行解析；
bool changePos(unsigned short sh)
{
		unsigned char parse;
		parse=sh&0x03;//判断内，面，角3种情况
		sh=sh>>2;
		if(parse==0)//00 内;	
		{
				printf("%s %d  \n","nei",sh&0x03);
		}
    	else if(parse==1)//01 面;
		{
				printf("%s %d  \n","mian",sh&0x03);
		}
    	else if(parse==2)//10 角;
		{
				printf("%s %d  \n","jiao",sh&0x03);
		}
    	else//解析失败
        	return false;
		sh=sh>>2;
		for(int i=0; i<2; i++) //x方向
		{
				if((sh&0x01)==1)
				{
						if(i==0)
						{
								printf("x--  \n"); 
						}
						else
						{
								printf("x++  \n");
						}
				}
				sh=sh>>1;
		}

		for(int i=0; i<2; i++) //y方向
		{
				if((sh&0x01)==1)
				{
						if(i==0)
						{
								printf("y--  \n");
						}
						else
						{
								printf("y++  \n"); 
						}
				}
				sh=sh>>1;
		}

		int temp=sh&0x01;//如果temp==1,则z++;temp==0,则z--;
		sh=sh>>1;

		for(int i=0; i<2; i++)
		{
				if( (sh&0x01)==1 )//是否进行z方向的行走;
				{
						if(i==0)
						{
               
								if(temp==1)
								{
                        
										printf("z++ ");
										if((sh&0x02)==2)
										{
												printf("layer++\n");
                            
										}
								}
								else
								{
                     
										printf("z--  ");
										if((sh&0x02)==2)
										{
												printf("layer--  \n");
                           
										}
								}
                
				}
				else //i==1
				{
						if(temp==1)
						{
								printf("z-- ");

								if((sh&0x02)==2)
								{
										printf("layer--\n");
								}
						}
						else
						{
								printf("z++ ");
								if((sh&0x02)==2)
								{
										printf("layer++\n");
								}
						}
				}
        }
       sh=sh>>2;
    } 
}

int main()
{
		unsigned short st[6];
		st[0]=0x02;
		st[1]=0x01;
		st[2]=0x3;
		st[3]=0x03;
		st[4]=0x7;
		st[5]=0x3;
		unsigned short i=setBits(st);
		changePos(i);
		putchar('\n');
}
