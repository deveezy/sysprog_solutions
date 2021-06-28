// #include <pthread.h>
// #include <stdio.h>
// #include <time.h>
// #include <stdlib.h>

// void *
// roll_dice()
// {
// 	int value = (rand() % 6) + 1;
// 	int *result = malloc(sizeof(int));
// 	*result = value;
// 	pthread_exit((void *)result);
// 	// return (void *) result;
// }

// int
// main(int argc, const char **argv)
// {
// 	int *res;
// 	srand(time(NULL));
// 	pthread_t th;
// 	pthread_create(&th, NULL, roll_dice, NULL);
// 	pthread_join(th, (void **)&res);
// 	printf("%d\n", *res);
// 	pthread_exit(0);
// 	free(res);
// 	return 0;
// }