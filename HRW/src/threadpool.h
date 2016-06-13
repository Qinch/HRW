#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<stdio.h>
#include<signal.h>
#include<errno.h>
#include<assert.h>

struct job
{
    void* (*callback_function)(void *arg);    //�̻߳ص�����
    void *arg;                                //�ص���������
    struct job *next;
};

struct threadpool
{
    int thread_num;                   //�̳߳�������̵߳ĸ���
    int queue_max_num;                //���������job�ĸ���
    struct job *head;                 //ָ��job��ͷָ��
    struct job *tail;                 //ָ��job��βָ��
    pthread_t *pthreads;              //�̳߳��������̵߳�pthread_t
    pthread_mutex_t mutex;            //������
    pthread_cond_t queue_not_empty;   //���в�Ϊ�յ���������
    pthread_cond_t queue_not_full;    //���в�Ϊ������������
    int queue_cur_num;                //���е�ǰ��job����
    int pool_close;                   //�̳߳��Ƿ��Ѿ��ر�
	int jobs_busy;
	pthread_cond_t jobs_stop;       //����Ϊ�յ���������
};

struct threadpool* threadpool_init(int thread_num, int queue_max_num);
int threadpool_add_job(struct threadpool *pool, void* (*callback_function)(void *arg), void *arg);
int threadpool_destroy(struct threadpool *pool);
void * threadpool_function(void* arg);
int threadpool_wait(struct threadpool *pool);

#endif
