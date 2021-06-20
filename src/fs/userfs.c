#include "userfs.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum {
	BLOCK_SIZE = 512,
	MAX_FILE_SIZE = 1024 * 1024 * 1024
};

/** Global error code. Set from any function on any error. */
static ufs_error_code ufs_err_code = UFS_ERR_NO_ERR;

typedef struct block {
	/** Block memory. */
	char *memory;
	/** How many bytes are occupied. */
	int occupied;
	/** Next block in the file. */
	struct block *next;
	/** Previous block in the file. */
	struct block *prev;
} block;

block * 
allocate_block(const char *const data, const size_t size)
{
	size_t mem_block_cnt = 1;
	size_t mem_block_remained = size % BLOCK_SIZE;
	size_t total_blocks = mem_block_cnt;
	if (size > BLOCK_SIZE) {
		mem_block_cnt = size / BLOCK_SIZE; 
		mem_block_remained = size % BLOCK_SIZE;
		total_blocks = mem_block_remained > 0 ? (mem_block_cnt + 1) : mem_block_cnt;
	}
	char *memory = malloc(total_blocks * BLOCK_SIZE);
	block *blocks = malloc(total_blocks * sizeof(block));
	memset(memory, 0, total_blocks * BLOCK_SIZE);
	memset(blocks, 0, total_blocks * sizeof(block));
	block *first_block = blocks;
	block *bl;
	for (size_t b = 0; b < total_blocks; ++b) {
		bl = blocks;
		bl->memory = memory;
		if (b != total_blocks - 1) {
			memcpy(bl->memory, data, BLOCK_SIZE);
			bl->occupied = BLOCK_SIZE;
		} else {
			memcpy(bl->memory, data, mem_block_remained);
			bl->occupied = mem_block_remained;
		}
		memory += BLOCK_SIZE;
		bl->next = ++blocks;
		block *cur = bl;	
		bl = bl->next;
		bl->prev = cur;	
	}
	return first_block;
}

struct file {
	/** Double-linked list of file blocks. */
	struct block *block_list;
	/**
	 * Last block in the list above for fast access to the end of file
	 */
	struct block *last_block;
	/** How many file descriptors are opened on the file. */
	int refs;
	/** File name. */
	const char *name;
	/** Files are stored in a double-linked list. */
	struct file *next;
	struct file *prev; 
};

static struct file *file_list = NULL;

struct filedesc {
	struct file *file;
	off_t offset;
	open_flags mode;
};

/**
 * An array of file descriptors. When a file descriptor is
 * created, its pointer drops here. When a file descriptor is
 * closed, its place in this array is set to NULL and can be
 * taken by next ufs_open() call.
 */
static struct filedesc **file_descriptors = NULL;
static int file_descriptor_count = 0;
static int file_descriptor_capacity = 0;

ufs_error_code
ufs_errno()
{
	return ufs_err_code;
}

int
ufs_open(const char *filename, int flags)
{
	
	//TODO: IMPLEMENT THIS FUNCTION
	ufs_err_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

ssize_t
ufs_write(int fd, const char *buf, size_t size)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_err_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

ssize_t
ufs_read(int fd, char *buf, size_t size)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_err_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

int
ufs_close(int fd)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_err_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

int
ufs_delete(const char *filename)
{
	/* IMPLEMENT THIS FUNCTION */
	ufs_err_code = UFS_ERR_NOT_IMPLEMENTED;
	return -1;
}

int
main(int argc, const char **argv) 
{
	char *mem = malloc(1550);
	mem[1549] = 0;
	memset(mem, 'x', 1549);
	block *b1 = allocate_block(mem, 1550);
	return 0;
}