#include<cstdio>
#include<cstdlib>
#include<cstring>
#include <getopt.h>
#include"argsparase.h"

bool args_m=false;//model file;
char args_model[ARGS_MAXLEN];

bool args_p=false;//powermap file;
char args_powermap[ARGS_MAXLEN];

bool args_a=false;//auto;
bool args_n=false;
int args_points;

bool args_c=false;
vector<double>cx;
vector<double>cy;

bool args_t=false;//转移区域表;
char args_table[ARGS_MAXLEN];

char* const short_options = (char* const)"m:p:an:c:t:h";
struct option long_options[] =
{
      	{"model", 1, NULL, 'm'},
      	{"power",1, NULL, 'p'},      
      	{"auto", 0, NULL, 'a'},
		{"number", 1, NULL, 'n'},        
      	{"coordinate", 1, NULL, 'c'}, 
		{"table", 1, NULL, 't'}, 
		{"help", 0, NULL, 'h'},             
      	{0, 0, 0, 0},      
};

inline void hint(char *app)
{
	printf("%s Usage:\n    选项:\n\t-m, --model, 指定model*.in文件;\n",app);
	printf("\t-p, --powermap, 指定powermap*.in文件;\n");
	printf("\t-a, --auto, 表示随机选取体积元进行计算,默认计算200个点;\n");
	printf("\t-n, --number, 当设置了'-a'时,用于设置随机计算的体积元的个数;\n");
	printf("\t-t, --table, 当设置了'-t'时,用于设置转移区域表;\n");
	printf("\t-c, --coordinate, 用于设置待计算坐标点的xy值(相对于该层芯片自身),多个坐标点间以','分隔,以回车表示输入结束;\n");
	printf("    例如:\n\t1 %s -m ./data/model1_1.in -p ./data/powermap1.in -a;\n \
			\r\t2 %s -m ./data/model1_1.in -p ./data/powermap1.in -t ./macros/table30.in -a -n 300 ;\n \
			\r\t3 %s -m ./data/model1_1.in -p ./data/powermap1.in -t ./macros/table30.in -c [x1,y1]\n \
			\r\t4 %s -m ./data/model1_1.in -p ./data/powermap1.in -t ./macros/table30.in -c [x1,y1] -c [x2,y2]\n \
			\r\t5 %s -m ./data/model1_1.in -p ./data/powermap1.in -t ./macros/table30.in -c [x1,y1],[x2,y2],[x3,y3]\n", \
											app,app,app,app,app);
}
void parasexy(char *str,char* app)
{
	FILE *stream=fmemopen(str, strlen (str), "r");
	char ch;
	do{
		if( (ch=fgetc(stream))!='[' )
		{
			fprintf(stderr,"%s:'-c'参数设置错误.\nTry '%s --help' for more information.\n",app,app);
			exit(149);
		}
		double tempx,tempy;
		if( (fscanf(stream,"%lf,%lf",&tempx,&tempy))!=2)
		{
			fprintf(stderr,"%s:'-c'参数设置错误.\nTry '%s --help' for more information.\n",app,app);
			exit(149);
		}
		
		cx.push_back(tempx);
		cy.push_back(tempy);
		//printf("%lf %lf\n",tempx,tempy);
		if( (ch=fgetc(stream))!=']')
		{
			fprintf(stderr,"%s:'-c'参数设置错误.\nTry '%s --help' for more information.\n",app,app);
			exit(149);
		}
		ch=fgetc(stream);
	}while((ch!=-1)&&(ch==','));
	if(ch!=-1)
	{
		fprintf(stderr,"%s:'-c'参数设置错误.\nTry '%s --help' for more information.\n",app,app);
		exit(149);
	}
}
void args_parase(int argc, char *argv[])      
{      
      	int ch;
      	while((ch= getopt_long (argc, argv, short_options, long_options, NULL)) != -1)      
      	{
         	switch (ch)
         	{
            	case 'm':
					strcpy(args_model,optarg);
					args_m=true;
               		break;
            	case 'p':
					strcpy(args_powermap,optarg);
					args_p=true;
               		break;
           		case 'a':
					args_a=true;
					if(!args_n)
						args_points=DEFTP;
              		break;
				case 'n':
					args_n=true;
					args_points=atoi(optarg);
					break;
				case'c':
					args_c=true;
					parasexy(optarg,argv[0]);
					break;
				case 't':
					args_t=true;
					strcpy(args_table,optarg);
					break;
				case'h':
					hint(argv[0]);
					exit(149);
            	default:
					fprintf(stderr,"Try '%s --help' for more information.\n",argv[0]);
					exit(149);
			
         	}
      	}
	if(  !( (args_m&&args_p&&args_a&&args_t&&(!args_c)) || (args_m&&args_p&&args_a&&args_n&&args_t&&(!args_c)) \
				|| (args_m&&args_p&&args_c&&args_t&&(!args_a)&&(!args_n)) )  )//参数设置错误;
	{
		fprintf(stderr,"error:参数设置错误.\n");	
		fprintf(stderr,"Try '%s --help' for more information.\n",argv[0]);
		exit(149);
	}

     	return ;
}
