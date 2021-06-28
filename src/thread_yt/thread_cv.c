// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <errno.h>

// pthread_mutex_t mtx;
// pthread_cond_t  cond;
// int fuel = 0;

// void *
// fuel_filling(void *arg)
// {
// 	for (int i = 0; i < 5; ++i) {
// 		pthread_mutex_lock(&mtx);
// 		fuel += 60;
// 		printf("Filling fuel...%d\n", fuel);
// 		pthread_mutex_unlock(&mtx);
// 		pthread_cond_broadcast(&cond);
// 		sleep(1);
// 	}
// }

// void *
// car(void *arg)
// {
// 	pthread_mutex_lock(&mtx);
// 	while (fuel < 40) {
// 		printf("Thread: %d, No fuel. \n", gettid());
// 		pthread_cond_wait(&cond, &mtx);
// 	}
// 	fuel -= 40;		
// 	printf("Thread: %d, Got fuel. Now left: %d\n", gettid(), fuel);
// 	pthread_mutex_unlock(&mtx);
// }

// int 
// main(int argc, const char **argv)
// {
// 	pthread_t th[5];
// 	pthread_mutex_init(&mtx, NULL);
// 	pthread_cond_init(&cond, NULL);
// 	for (int i = 0; i < 5; ++i) {
// 		if (i == 4) {
// 			pthread_create(&th[i], NULL, fuel_filling, NULL);
// 		} else {
// 			pthread_create(&th[i], NULL, car, NULL);
// 		}
// 	}
// 	for (int i = 0; i < 5; ++i) {
// 		pthread_join(th[i], NULL); 
// 	}
// 	pthread_mutex_destroy(&mtx);
// 	pthread_cond_destroy(&cond);
// 	return 0;
// }