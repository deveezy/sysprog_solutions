#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

// 1) close newfd, now newfd is added to free list of fds 
// 2) dup oldfd, dup returns the smallest fd from the free list of fds
// 3) we continue dup while (fd != newfd)

typedef struct {
	int *data;
	size_t size;
	size_t capacity;
} Stack;

static Stack *
stack_new(void)
{
	Stack *s; 
	size_t full_size = sizeof(*s) + 64 * sizeof(int);
	s = (Stack *)malloc(full_size);
	assert(s != NULL);
	s->size = 0;
	s->capacity = 64;
	s->data = (int *)((char *)s + sizeof(*s));
	assert(s->data != NULL);
	return s;
}

static int
stack_size(const Stack *const s)
{
	return s->size;
}

static void
stack_push(Stack *const s, const int item)
{
	if (s->size == s->capacity) {
		size_t new_cap = s->capacity * 2;
		s->data = realloc(s, sizeof(*s) + s->capacity * sizeof(int));
		s->capacity = new_cap;
		assert(s->data != NULL);
	}
	s->data[s->size++] = item;
}

static int
stack_pop(Stack *const s)
{
	return s->data[s->size--];
}

static void
stack_destroy(Stack **s)
{
	free(*s);
	*s = NULL;
}

static int
dup2_impl(const int oldfd, const int newfd)
{
	if (newfd < 0 || oldfd < 0) {
		errno = EBADF; /* Bad file number */
		return -1;
	}

	if (oldfd == newfd)
		return newfd;
	
	int fd = dup(oldfd);

	if (fd < 0) {
		errno = EBADF;
		return -1;
	}

	// oldfd is valid
	close(fd);

	// Silently ignore EBADF
	if (close(newfd) < 0 && errno != EBADF) {
		// errno will be set by close
		return -1;
	}

	Stack *s = stack_new();

	while ((fd = dup(oldfd)) != newfd) {
		stack_push(s, fd);
	}

	// Now newfd is a dup of oldfd, and the stack contains all the
	// intermediate dups
	while (stack_size(s) > 0) {
		fd = stack_pop(s);
		close(fd);
	}

	stack_destroy(&s);
	return newfd;
}