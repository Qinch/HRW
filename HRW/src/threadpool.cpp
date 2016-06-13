#include "threadpool.h"

//thread_num�̳߳����̵߳������������е����job����;
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
        pool->thread_num = thread_num;//�̳߳����̵߳ĸ���;
        pool->queue_max_num = queue_max_num;//��Ϣ���еĴ�С
        pool->queue_cur_num = 0;
        pool->head = NULL;
        pool->tail = NULL;
		pool->jobs_busy=0;
        if (pthread_mutex_init(&(pool->mutex), NULL))//��ʼ��������;
        {
            printf("failed to init mutex!\n");
            break;
        }
        if (pthread_cond_init(&(pool->jobs_stop), NULL))//��ʼ����������;
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
        pool->pthreads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);//�߳�����;
        if (NULL == pool->pthreads)
        {
            printf("failed to malloc pthreads!\n");
            break;
        }
        pool->pool_close = 0;//�ڴ��δ�ر�;
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

    pthread_mutex_lock(&(pool->mutex));//����;
    while ((pool->queue_cur_num == pool->queue_max_num) && !( pool->pool_close))
    {
        pthread_cond_wait(&(pool->queue_not_full), &(pool->mutex));   //��������ʱ��͵ȴ�
    }
    if ( pool->pool_close)    //���йرջ����̳߳عرվ��˳�
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
    pthread_cond_signal(&(pool->queue_not_empty));  //��������ʱ��֪ͨ�̳߳��е��̣߳����зǿ� 
    return 0;
}

void* threadpool_function(void* arg)
{
    struct threadpool *pool = (struct threadpool*)arg;
    struct job *pjob = NULL;
    while (1)  //��ѭ��
    {
        pthread_mutex_lock(&(pool->mutex));
        while ((pool->queue_cur_num == 0) && !pool->pool_close)   //����Ϊ��ʱ���͵ȴ����зǿ�
        {
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->mutex));
        }
        if (pool->pool_close)   //�̳߳عرգ��߳̾��˳�
        {
            pthread_mutex_unlock(&(pool->mutex));
            pthread_exit(NULL);
        }
        pool->queue_cur_num--;//�Ӷ�����ȡ��һ��job;
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
        pthread_cond_signal(&(pool->queue_not_full));  //���з������Ϳ���֪ͨthreadpool_add_job���������������
        
        (*(pjob->callback_function))(pjob->arg);   //�߳�����Ҫ���Ĺ������ص������ĵ���
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
        pthread_cond_wait(&(pool->jobs_stop), &(pool->mutex));  //�ȴ�����Ϊ��
    }
    pthread_mutex_unlock(&(pool->mutex));

    return 0;
}

int threadpool_destroy(struct threadpool *pool)
{
    assert(pool != NULL);
    pthread_mutex_lock(&(pool->mutex));
    if(pool->pool_close)   //�̳߳��Ѿ��˳��ˣ���ֱ�ӷ���
    {
        pthread_mutex_unlock(&(pool->mutex));
        return -1;
    }
    
    pool->pool_close = 1;      //���̳߳عرձ�־
    pthread_mutex_unlock(&(pool->mutex));

    pthread_cond_broadcast(&(pool->queue_not_empty));  //�����̳߳��������������߳�
    pthread_cond_broadcast(&(pool->queue_not_full));   //������������threadpool_add_job����
    int i;
    for (i = 0; i < pool->thread_num; ++i)
    {
        pthread_join(pool->pthreads[i], NULL);    //�ȴ��̳߳ص������߳�ִ�����
    }
    
    pthread_mutex_destroy(&(pool->mutex));          //������Դ
    pthread_cond_destroy(&(pool->jobs_stop));
    pthread_cond_destroy(&(pool->queue_not_empty));   
    pthread_cond_destroy(&(pool->queue_not_full));    
    free(pool->pthreads);
    free(pool);

    return 0;
}
