#ifndef ARGSPARASE_H_
#define ARGSPARASE_H_

#include<vector>
using std::vector;

#define DEFTP 200 //默认自动测试体积元的个数;
#define ARGS_MAXLEN 1024

extern char* const short_options;
extern struct option long_options[];

extern bool args_m;//model文件;
extern char args_model[ARGS_MAXLEN];

extern bool args_p;//powermap文件;
extern char args_powermap[ARGS_MAXLEN];

extern bool args_a;//表示自动计算,默认计算２００个点;
extern bool args_n;//表示计算的体积元的个数
extern int args_points;

extern bool args_c;
extern vector<double>cx;//用于存储手工输入的测试所有的xy坐标（相对于该层芯片自身）;
extern vector<double>cy;

extern bool args_t;//转移区域表;
extern char args_table[ARGS_MAXLEN];

void hint(char *app);
void parasexy(char *str,char* app);
void args_parase(int argc,char** argv);

#endif /* ARGUMENTS_H_ */
