// #include <pthread.h>
// #include <stdio.h>

// void *
// routine(void *arg)
// {
// 	printf("Test from thread: %d\n",*((int *)arg));
// 	return NULL;
// }

// int 
// main(int argc, const char **argv)
// {
// 	pthread_t t1, t2, t3, t4, t5;
// 	int arg = 101;
// 	pthread_create(&t1, NULL, routine, &arg);
// 	pthread_create(&t2, NULL, routine, &arg);
// 	pthread_create(&t3, NULL, routine, &arg);
// 	pthread_create(&t4, NULL, routine, &arg);
// 	pthread_create(&t5, NULL, routine, &arg);
// 	pthread_join(t1, NULL);
// 	pthread_join(t2, NULL);
// 	pthread_join(t3, NULL);
// 	pthread_join(t4, NULL);
// 	pthread_join(t5, NULL);
// 	return 0;
// }