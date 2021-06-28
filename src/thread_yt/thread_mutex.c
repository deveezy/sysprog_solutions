// #include <pthread.h>
// #include <stdio.h>
// #include <time.h>
// #include <stdlib.h>

// int primes[10] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29 };

// void *
// routine(void *arg)
// {
// 	int index = *(int *)arg;
// 	int sum = 0;
// 	for (int j = 0; j < 5; ++j) {
// 		sum += primes[index + j];
// 	}
// 	*(int *)arg = sum;
// 	return arg;
// }

// int
// main(int argc, const char **argv)
// {
// 	int *first = malloc(sizeof(int));
// 	int *second = malloc(sizeof(int));
// 	*first = 0;
// 	*second = 5;
// 	pthread_t th[2];
// 	pthread_create(&th[0], NULL, routine, first);
// 	pthread_create(&th[1], NULL, routine, second);

// 	int sum = 0;
// 	for (int i = 0; i < 2; ++i) {
// 		int *r;
// 		pthread_join(th[i], (void **)&r);
// 		sum += *r;
// 		free(r);
// 	}
// 	printf("%d\n", sum);
// 	return 0;
// }