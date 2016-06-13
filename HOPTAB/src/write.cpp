#include<stdlib.h>
#include"random.h"
#include"macrostruct.h"

int main(int argc,char **argv)
{
		Random rand;
		Macros macros;
		macros.ratio=atof(argv[1]);
		macros.size[0]=250.5;
		macros.size[1]=250.5;
		macros.size[2]=250.5;
		macros.dzbound=12.5e-6;//1e-6;//5e-6;
		macros.mediums.push_back(30);
		macros.mediums.push_back(125);
		macros.mediums.push_back(395);
		//macros.h.push_back(8.7e3);
		
		init(macros);
		walk(Pow(2,20),macros,rand);//20
		FILE *fp=fopen(argv[2],"w");
		write(macros,fp);
		fclose(fp);
}
