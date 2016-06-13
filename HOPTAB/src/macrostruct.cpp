#include <cstdio>
#include<cmath>
#include "macrostruct.h"

#define EPS 1e-3

bool dict[3][3]={{false,true,true},{true,false,false},{true,false,false}};

void init(Macros& mac)
{
		mac.inner=new vector<double> [3];
		
		mac.intersect=new vector<double>**[3];
		for(int i=0;i<3;i++)
		{
			mac.intersect[i]=new vector<double>*[3];
			for(int j=0;j<3;j++)
			{
				if(dict[i][j]==true)
					mac.intersect[i][j]=new vector<double>[3];
				else
					mac.intersect[i][j]=NULL;
			}
		}
		mac.bound=new vector<double>[3];
}

//�ֱ�vec����Ĵ�С,vec���������д���ļ�;
void savevec(FILE* outfile,vector<double>& vec )
{
		int size=vec.size();
		fwrite(&size,sizeof(int),1,outfile);
		fwrite(&(vec[0]),sizeof(double),size,outfile);
		return ;
}

//�Ӷ������ļ���ȡvec���飬����vec����;
void loadvec(FILE* infile,vector<double>& vec )
{
		int size;
		vec.clear();
		fread(&size,sizeof(int),1,infile);//��ȡvec����Ĵ�С;
		if (size==0) 
				return;
		vec.resize(size);//����vec����Ĵ�С;
		fread(&(vec[0]),sizeof(double),size,infile);//��ȡvec���������;
		return ;
}

//��ȡת�������ļ�;
/*void read(Macros& macros, FILE* f)
{
		fread(&(macros.ratio),sizeof(double),1,f);//���Ԫ�Ŀ�߱�;
		fread((macros.size),sizeof(double),3,f);//lx,ly,lz;
		fread(&(macros.dzbound),sizeof(double),1,f);//���Ԫ�ĸ߶ȣ�
		loadvec(f,macros.mediums);//�ȵ�������;
		loadvec(f,macros.h);//�ȴ���ϵ������;

		macros.inner=new vector<double> [3];//ת�Ƹ�������;
		for (int i=0;i<3;i++)//��ȡͬһ�ֲ����ڲ�������ת�Ʊ�;
		{
				loadvec(f,macros.inner[i]);
		}
	
		for (int i=0;i<4;i++)//����оƬ������ת�Ʊ����������±�0:01��; 1:01��; 2:02��; 3:02��;
		{
				macros.intersect[i]=new vector<double>[3];
				for (int k=0;k<3;k++)//0:x;1:y;2:z;
				{
						loadvec(f,macros.intersect[i][k]);
				}
		}
		
		macros.bound=new vector<double>[3];//�����߽�ת�������;
		for (int k=0;k<3;k++)
		{
				loadvec(f,macros.bound[k]);
		}
}
*/
void write(Macros& macros, FILE* f)
{
		fwrite(&(macros.ratio),sizeof(double),1,f);
		fwrite((macros.size),sizeof(double),3,f);
		fwrite(&(macros.dzbound),sizeof(double),1,f);
		savevec(f,macros.mediums);
		savevec(f,macros.h);
		for (int i=0;i<3;i++)//ͬһ�ֲ���;
		{
				savevec(f,macros.inner[i]);
		}

		for (int i=0;i<3;i++)
		{
				for (int j=0;j<3;j++)
				{
					if(macros.intersect[i][j]!=NULL)
						for(int k=0;k<3;k++)
							savevec(f,macros.intersect[i][j][k]);
				}
		}

		for (int k=0;k<3;k++)
			savevec(f,macros.bound[k]);
}

int Pow(int x,int n)
{
	if(n==0)
		return 1;
	if(n==1)
		return x;
	if(n%2==0)
		return Pow(x*x,n/2);
	else
		return x*Pow(x*x,n/2);
}

void walkinner(int walks,vector<double>* inner,Macros& macros,Random &rand)
{
		double prop=macros.ratio;	
		double lx=macros.size[0];
		double ly=macros.size[1];
		double lz=macros.size[2];
		
		int count=0;
		double probxy=1/(4.0+2.0*prop*prop);//��xy���������ߵĵĸ���;
		double probz=1/(2+4/prop/prop);
		for (int i=0;i<walks;i++)
		{
				if (i%(walks/100)==0) 
				{
						printf("Walked %d%%\n",count);//������;
						count++;
				}
				double x=0,y=0,z=0;//���������ʼ�����;
				while (true)
				{
						if (fabs(lx-0.5-x)<EPS) 
								break;
						if (fabs(-lx+0.5-x)<EPS) 
								break;
						if (fabs(ly-0.5-y)<EPS) 
								break;
						if (fabs(-ly+0.5-y)<EPS) 
								break;
						if (fabs(lz-0.5-z)<EPS) 
								break;
						if (fabs(-lz+0.5-z)<EPS) 
								break;
						
						double rnd=rand.getrand();
						if((rnd-=probxy)<0)
							x+=1;
						else if((rnd-=probxy)<0)
							x-=1;
						else if((rnd-=probxy)<0)
							y+=1;
						else if((rnd-=probxy)<0)
							y-=1;
						else if((rnd-=probz)<0)
							z+=1;
						else
							z-=1;
				}
				inner[0].push_back(x/lx);
				inner[1].push_back(y/ly);
				inner[2].push_back(z/lz);
	 }
}

void walkintersect(int walks,vector<double>*intersect,Macros& macros,double medl,double medh,Random &rand)
{
		double prop=macros.ratio;
		double lx=macros.size[0];
		double ly=macros.size[1];
		double lz=macros.size[2];
	
		//λ���ϲ�оƬ�ĸ��ʣ�
		
		int count=0;
		double zplus=medh/(medl+medh);//����оƬ������ֻ����z���������;
		double pxy=1/(4.0+2.0*prop*prop);//������оƬ������;
		double pz=1/(2+4/prop/prop);

		for (int i=0;i<walks;i++)//�ϲ�оƬ
		{
				if (i%(walks/100)==0) 
				{
						printf("Walked(high) %d%%\n",count);
						count++;
				}
				double x=0,y=0,z=0;//���������ʼ�����;
				while (true)
				{
						if (fabs(lx-0.5-x)<EPS) 
								break;
						if (fabs(-lx+0.5-x)<EPS) 
								break;
						if (fabs(ly-0.5-y)<EPS) 
								break;
						if (fabs(-ly+0.5-y)<EPS) 
								break;
						if (fabs(lz-0.5-z)<EPS) 
								break;
						if (fabs(-lz+0.5-z)<EPS) 
								break;
						
						double rnd=rand.getrand();
						if(fabs(z)<EPS)//λ�ڽ����棬ֻ����z���������;
						{
								if((rnd-=zplus)<0)
										z+=1;
								else 
										z-=1;
						}
						else
						{
								if((rnd-=pxy)<0)
										x+=1;
								else if((rnd-=pxy)<0)
										x-=1;
								else if((rnd-=pxy)<0)
										y+=1;
								else if((rnd-=pxy)<0)
										y-=1;
								else if((rnd-=pz)<0)
										z+=1;
								else
										z-=1;
						}
				}
				intersect[0].push_back(x/lx);
				intersect[1].push_back(y/ly);
				intersect[2].push_back(z/lz);

		}
}

/*void walkconv(int walks,vector<double>* conv,Macros& macros,int idx1,int idx2)//ֻ�洢��������棬���洢�����߽�;
{
		double prop=macros.ratio;// prop=dx/dz  ���Ԫ��߱�;
		double lx=macros.size[0];
		double ly=macros.size[1];
		double lz=500.5;//macros.size[2];		
		int count=0;
		//�Ƕ����߽�;
		double probxy=1/(4.0+2.0*prop*prop);//��xy���������ߵĵĸ���;
		double probz=1/(2+4/prop/prop);

		double pout=(macros.h[idx2])/((macros.mediums[idx1]/macros.dzbound)+macros.h[idx2]);//�����߽�ֻ����z���������;
		for (int i=0;i<walks;i++)
		{
				if (i%(walks/100)==0) 
				{
						printf("Walked %d%%\n",count);
						count++;
				}

				double x=0,y=0,z=0;//���������ʼ�����;
				while (true)
				{
						if (fabs(lx-0.5-x)<EPS) 
							break;
						if (fabs(-lx+0.5-x)<EPS) 
							break;
						if (fabs(ly-0.5-y)<EPS) 
							break;
						if (fabs(-ly+0.5-y)<EPS) 
							break;
						if (fabs(-lz+0.5-z)<EPS) 
							break;

						if (fabs(lz-0.5-z)<EPS)//λ�ڶ����߽���ֻ����z�����������ߡ�
						{
								if (getrand()<pout) 
								{
										z+=10;
										break;
								}
								else
										z-=1;
						}
						else//������������xyz���������
						{
								double rnd=getrand();
								if((rnd-=probxy)<0)
										x+=1;
								else if((rnd-=probxy)<0)
										x-=1;
								else if((rnd-=probxy)<0)
										y+=1;
								else if((rnd-=probxy)<0)
										y-=1;
								else if((rnd-=probz)<0)
										z+=1;
								else
										z-=1;
						}
				}
				if ( lz<z )//�����������߽磬�򲻴洢xyz����;
						continue;
				conv[0].push_back( x/lx);
				conv[1].push_back( y/ly);
				conv[2].push_back( z/lz);
		}	
}*/

void walkconv(int walks,vector<double>* conv,Macros& macros,int idx1,int idx2,Random &rand)//ֻ�洢��������棬���洢�����߽�;
{
		double prop=macros.ratio;// prop=dx/dz  ���Ԫ��߱�;
		double lx=macros.size[0];
		double ly=macros.size[1];
		double lz=100.5;//macros.size[2];
		
		int count=0;
		//�Ƕ����߽�;
		double probxy=1/(4.0+2.0*prop*prop);//��xy���������ߵĵĸ���;
		double probz=1/(2+4/prop/prop);

		double pout=(macros.h[idx2])/((macros.mediums[idx1]/macros.dzbound)+macros.h[idx2]);//�����߽�ֻ����z���������;
		for (int i=0;i<walks;i++)
		{
				if (i%(walks/100)==0) 
				{
						printf("Walked %d%%\n",count);
						count++;
				}

				double x=0,y=0,z=0;//���������ʼ�����;
				while (true)
				{
						if (fabs(lx-0.5-x)<EPS) 
							break;
						if (fabs(-lx+0.5-x)<EPS) 
							break;
						if (fabs(ly-0.5-y)<EPS) 
							break;
						if (fabs(-ly+0.5-y)<EPS) 
							break;
						if (fabs(-lz+0.5-z)<EPS) 
								break;

						if (fabs(lz-0.5-z)<EPS)//λ�ڶ����߽���ֻ����z�����������ߡ�
						{
								if (rand.getrand()<pout) 
								{
										z+=1;
										break;
								}
								else
										z-=1;
						}
						else//������������xyz���������
						{
								double rnd=rand.getrand();
								if((rnd-=probxy)<0)
										x+=1;
								else if((rnd-=probxy)<0)
										x-=1;
								else if((rnd-=probxy)<0)
										y+=1;
								else if((rnd-=probxy)<0)
										y-=1;
								else if((rnd-=probz)<0)
										z+=1;
								else
										z-=1;
						}
				}
				if ( lz<z )//�����������߽磬�򲻴洢xyz����;
						continue;
				conv[0].push_back( x/lx);
				conv[1].push_back( y/ly);
				conv[2].push_back( z/lz);
		}	
}

void walk(int walks,Macros& macros,Random &rand)
{
	printf("Walking inner:\n");
	walkinner(walks,macros.inner,macros,rand);

	printf("Walking intersect:\n");
	for(int i=0;i<3;i++)
		for(int j=0;j<3;j++)
			if(macros.intersect[i][j]!=NULL)
				walkintersect(walks,macros.intersect[i][j],macros,macros.mediums[i],macros.mediums[j],rand);
	//printf("Walking boundary:\n");
	//walkconv(walks,macros.bound,macros,2,0,rand);
}
