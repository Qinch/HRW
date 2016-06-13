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
    void* (*callback_function)(void *arg);    //线程回调函数
    void *arg;                                //回调函数参数
    struct job *next;
};

struct threadpool
{
    int thread_num;                   //线程池中最大线程的个数
    int queue_max_num;                //队列中最大job的个数
    struct job *head;                 //指向job的头指针
    struct job *tail;                 //指向job的尾指针
    pthread_t *pthreads;              //线程池中所有线程的pthread_t
    pthread_mutex_t mutex;            //互斥锁
    pthread_cond_t queue_not_empty;   //队列不为空的条件变量
    pthread_cond_t queue_not_full;    //队列不为满的条件变量
    int queue_cur_num;                //队列当前的job个数
    int pool_close;                   //线程池是否已经关闭
	int jobs_busy;
	pthread_cond_t jobs_stop;       //队列为空的条件变量
};

struct threadpool* threadpool_init(int thread_num, int queue_max_num);
int threadpool_add_job(struct threadpool *pool, void* (*callback_function)(void *arg), void *arg);
int threadpool_destroy(struct threadpool *pool);
void * threadpool_function(void* arg);
int threadpool_wait(struct threadpool *pool);

#endif
