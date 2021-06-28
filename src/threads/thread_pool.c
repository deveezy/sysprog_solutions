#include "thread_pool.h"
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct thread_task {
	thread_task_f function;
	void *arg;
	void *ret;
	volatile bool is_finished;
	volatile bool is_running;
} thread_task;

typedef struct thread_pool {
	pthread_t *threads;
	thread_task **queue;
	pthread_mutex_t queue_mtx;
	int threads_size;
	int thread_count;
	int queue_size;
	int queue_head;
	int queue_tail;
	int queue_count;	
} thread_pool;

static void *
process_task(void *arg)
{
	thread_pool *pool = (thread_pool *)arg;
	thread_task *task;
	unsigned long counter = 0;
	for (;;) {
		pthread_mutex_lock(&pool->queue_mtx);
		if (pool->queue_count > 0) {
			task = pool->queue[pool->queue_head];
			pool->queue_head += 1;
			pool->queue_count -= 1;
			task->is_running = true;
			void *res = (*(task->function))(task->arg);
			task->ret = res;
			task->is_finished = true;
			task->is_running = false;
		}
		pthread_mutex_unlock(&pool->queue_mtx);
		++counter;
		if (counter == 1000000000) {
			pthread_exit(0);
		}
	}
}

int
thread_pool_new(int max_thread_count, thread_pool **pool)
{
	if (max_thread_count > TPOOL_MAX_THREADS || max_thread_count < 1)
		return TPOOL_ERR_INVALID_ARGUMENT;
	thread_pool *tp = malloc(sizeof(*tp));
	memset(tp, 0, sizeof(*tp));
	tp->threads = malloc(sizeof(pthread_t) * max_thread_count);
	thread_task **tasks = malloc(sizeof( thread_task *) * TPOOL_MAX_TASKS);
	memset(tasks, 0, sizeof( thread_task *) * TPOOL_MAX_TASKS);
	pthread_mutex_init(&tp->queue_mtx, NULL);
	tp->threads_size = max_thread_count;
	tp->queue = tasks;
	tp->queue_size = TPOOL_MAX_TASKS;
	tp->queue_count = 0;
	tp->queue_head = 0;
	tp->queue_tail = 0;
	
	*pool = tp;
	return 0;
}

int
thread_task_execute(thread_pool *tp)
{
  for (size_t i = 0; i < tp->threads_size; ++i) {
    if (!pthread_create(&tp->threads[i], NULL, process_task, (void *) tp))
      ++tp->thread_count;
  }
  if (tp->thread_count != tp->threads_size) {
    free(tp);
    free(tp->threads);
    return TPOOL_ERR_INVALID_ARGUMENT; // change enum value
  }
	return 0;
}

void thread_pool_join(thread_pool *pool) 
{
	for (size_t i = 0; i < pool->thread_count; ++i) {
		pthread_join(pool->threads[i], NULL);
	}
}

int
thread_pool_thread_count(const struct thread_pool *pool)
{
	assert(pool != NULL);
	if (pool != NULL) {
		return pool->thread_count;
	}
	return -1;
}

int
thread_pool_delete(thread_pool **pool)
{
	assert(pool != NULL || *pool != NULL);
	if (pool != NULL || *pool != NULL) {
		pthread_mutex_destroy(&((*pool)->queue_mtx));
		free((*pool)->threads);
		free((*pool)->queue);
		free(*pool);
		*pool = NULL;
		return 0;
	}
	return TPOOL_ERR_INVALID_ARGUMENT; 
}

int
thread_pool_push_task(thread_pool *pool, thread_task *task)
{
	assert(pool != NULL);
	assert(task != NULL);
	if (!pool || !task) {
		return TPOOL_ERR_INVALID_ARGUMENT;
	}
	if (pool->thread_count + 1 == TPOOL_MAX_TASKS) {
		return TPOOL_ERR_TOO_MANY_TASKS;
	}
	pthread_mutex_lock(&pool->queue_mtx);
	pool->queue[pool->queue_tail] = task;
	pool->queue_tail += 1;
	pool->queue_count += 1;
	pthread_mutex_unlock(&pool->queue_mtx);

	return 0;
}

int
thread_task_new(thread_task **task, thread_task_f function, void *arg)
{
	thread_task *t_task = malloc(sizeof(*t_task));
	memset(t_task, 0, sizeof(*t_task));
	t_task->function = function;
	t_task->arg = arg;
	t_task->is_finished = false;
	t_task->ret = NULL;
	t_task->is_running = false;
	*task = t_task;
	return 0;
}

bool
thread_task_is_finished(const thread_task *task)
{
	return task->is_finished;
}

bool
thread_task_is_running(const thread_task *task)
{
	return task->is_running;
}

int
thread_task_join(thread_task *task, void **result)
{
	if (task != NULL) {
		while (!task->is_finished || task->is_running) {}
		int *test = malloc(sizeof(int));
		*test = *((int*)task->ret);
		free(task->ret);
		*result = test;
		// thread_task_delete(&task);
		return 0;
	}
	return TPOOL_ERR_INVALID_ARGUMENT; 
}

// delete task memory when took the result.
int
thread_task_delete(thread_task **task)
{
	if ((*task)->is_finished) {
		assert(task != NULL || *task != NULL);
		if (task != NULL || *task != NULL) {
			free(*task);
			*task = NULL;
		}
		return 0;
	}
	return TPOOL_ERR_HAS_TASKS;
}

int
thread_task_detach(thread_task *task)
{
	/* IMPLEMENT THIS FUNCTION */
	return TPOOL_ERR_NOT_IMPLEMENTED;
}

void *
test_ret(void *arg)
{
	int *res = malloc(sizeof(int));
	*res = *(int *)arg;
	*res *= *res;
	return res;
}

int main()
{
	thread_pool *pool;
	int res = thread_pool_new(4, &pool);
	thread_task *task;
	int arg = 10;
	thread_task_new(&task, (thread_task_f) test_ret, &arg);
	int *res_arg;
	thread_pool_push_task(pool, task);
	thread_task_execute(pool);
	thread_task_join(task, (void **)&res_arg);
	thread_task_delete(&task);
	free(res_arg);
	thread_pool_join(pool);
	thread_pool_delete(&pool);
	return 0;
}