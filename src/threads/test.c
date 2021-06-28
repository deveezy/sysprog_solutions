#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread_pool.h"

void * 
print_message(const char *str)
{
	printf("%s\n", str);
	return NULL;
}
