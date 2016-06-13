#include "threadpool.h"

//thread_num线程池中线程的最大个数，队列的最大job个数;
struct threadpool* threadpool_init(int thread_num, int queue_max_num)
{
    struct threadpool *pool = NULL;
    do 
    {
        pool =(threadpool*) malloc(sizeof(struct threadpool));
        if (NULL == pool)
        {
            printf("failed to malloc threadpool!\n");
            break;
        }
        pool->thread_num = thread_num;//线程池中线程的个数;
        pool->queue_max_num = queue_max_num;//消息队列的大小
        pool->queue_cur_num = 0;
        pool->head = NULL;
        pool->tail = NULL;
		pool->jobs_busy=0;
        if (pthread_mutex_init(&(pool->mutex), NULL))//初始化互斥锁;
        {
            printf("failed to init mutex!\n");
            break;
        }
        if (pthread_cond_init(&(pool->jobs_stop), NULL))//初始化条件变量;
        {
            printf("failed to init queue_empty!\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_empty), NULL))
        {
            printf("failed to init queue_not_empty!\n");
            break;
        }
        if (pthread_cond_init(&(pool->queue_not_full), NULL))
        {
            printf("failed to init queue_not_full!\n");
            break;
        }
        pool->pthreads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);//线程数组;
        if (NULL == pool->pthreads)
        {
            printf("failed to malloc pthreads!\n");
            break;
        }
        pool->pool_close = 0;//内存池未关闭;
        int i;
        for (i = 0; i < pool->thread_num; ++i)
        {
            pthread_create(&(pool->pthreads[i]), NULL, threadpool_function, (void *)pool);
        }
		
        return pool;    
    } while (0);
    
    return NULL;
}

int threadpool_add_job(struct threadpool* pool, void* (*callback_function)(void *arg), void *arg)
{
    assert(pool != NULL);
    assert(callback_function != NULL);
    assert(arg != NULL);

    pthread_mutex_lock(&(pool->mutex));//加锁;
    while ((pool->queue_cur_num == pool->queue_max_num) && !( pool->pool_close))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));   //队列满的时候就等待
    }
    if ( pool->pool_close)    //队列关闭或者线程池关闭就退出
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    struct job *pjob =(struct job*) malloc(sizeof(struct job));
    if (NULL == pjob)
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    } 
    pjob->callback_function = callback_function;    
    pjob->arg = arg;
    pjob->next = NULL;
    if (pool->queue_cur_num==0)   
    {
        pool->head = pool->tail = pjob;
    }
    else
    {
        pool->tail->next = pjob;
        pool->tail = pjob;    
    }
    pool->queue_cur_num++;
	pool->jobs_busy++;
    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_signal(&(pool->queue_not_empty));  //有任务来时就通知线程池中的线程：队列非空 
    return 0;
}

void* threadpool_function(void* arg)
{
    struct threadpool *pool = (struct threadpool*)arg;
    struct job *pjob = NULL;
    while (1)  //死循环
    {
        pthread_mutex_lock(&(pool->mutex));
        while ((pool->queue_cur_num == 0) && !pool->pool_close)   //队列为空时，就等待队列非空
        {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }
        if (pool->pool_close)   //线程池关闭，线程就退出
        {
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        pool->queue_cur_num--;//从队列中取走一个job;
        pjob = pool->head;
		
        if (pool->queue_cur_num == 0)
        {
            pool->head = pool->tail = NULL;
        }
        else 
        {
            pool->head = pjob->next;
        }
        pthread_mutex_unlock(&(pool->mutex));
        pthread_cond_signal(&(pool->queue_not_full));  //队列非满，就可以通知threadpool_add_job函数，添加新任务
        
        (*(pjob->callback_function))(pjob->arg);   //线程真正要做的工作，回调函数的调用
        free(pjob);
        pjob = NULL;

		pthread_mutex_lock(&(pool->mutex));
		pool->jobs_busy--;
		if(pool->jobs_busy==0)
		{
			pthread_cond_signal(&(pool->jobs_stop));
		}
		pthread_mutex_unlock(&(pool->mutex));

    }
}
int threadpool_wait(struct threadpool *pool)
{
    assert(pool != NULL);
    pthread_mutex_lock(&(pool->mutex));
    while (pool->jobs_busy!= 0)
    {
        pthread_cond_wait(&(pool->jobs_stop), &(pool->mutex));  //等待队列为空
    }
    pthread_mutex_unlock(&(pool->mutex));

    return 0;
}

int threadpool_destroy(struct threadpool *pool)
{
    assert(pool != NULL);
    pthread_mutex_lock(&(pool->mutex));
    if(pool->pool_close)   //线程池已经退出了，就直接返回
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    
    pool->pool_close = 1;      //置线程池关闭标志
    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_broadcast(&(pool->queue_not_empty));  //唤醒线程池中正在阻塞的线程
    pthread_cond_broadcast(&(pool->queue_not_full));   //唤醒添加任务的threadpool_add_job函数
    int i;
    for (i = 0; i < pool->thread_num; ++i)
    {
        pthread_join(pool->pthreads[i], NULL);    //等待线程池的所有线程执行完毕
    }
    
    pthread_mutex_destroy(&(pool->mutex));          //清理资源
    pthread_cond_destroy(&(pool->jobs_stop));
    pthread_cond_destroy(&(pool->queue_not_empty));   
    pthread_cond_destroy(&(pool->queue_not_full));    
    free(pool->pthreads);
    free(pool);

    return 0;
}
