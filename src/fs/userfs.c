#include "userfs.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

enum {
	BLOCK_SIZE = 8,
	MAX_FILE_SIZE = 1024 * 1024 * 1024
};

void
print_err_msg(ufs_error_code err_code) 
{
	switch (err_code) {
	case UFS_ERR_NO_FILE:
		fprintf(stderr, "Error: No file\n");
		break;
	case UFS_ERR_NO_MEM:
		fprintf(stderr, "Error: No memory\n");
		break;
	case UFS_ERR_NO_PERMISSION:
		fprintf(stderr, "Error: No permission\n");
		exit(1);
	default:
		fprintf(stderr, "Error: Incorrect error number\n");
		break;
	}
}

/** Global error code. Set from any function on any error. */
static ufs_error_code ufs_err_code = UFS_ERR_NO_ERR;

struct block {
	/** Block memory. */
	char *memory;
	/** How many bytes are occupied. */
	int occupied;
	/** Next block in the file. */
	struct block *next;
	/** Previous block in the file. */
	struct block *prev;
};

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

size_t  write_data(struct file *file, const char *data, const size_t size);
size_t  read_data(struct file *file, const char *buf, const size_t size);
struct file * alloc_file_struct(const char *const filename);

static struct file *file_list = NULL;

struct filedesc {
	struct file *file;
	off_t offset;
	int mode;
};

/**
 * An array of file descriptors. When a file descriptor is
 * created, its pointer drops here. When a file descriptor is
 * closed, its place in this array is set to NULL and can be
 * taken by next ufs_open() call.
 */
static struct filedesc **file_descriptors = NULL;
static int file_descriptor_count = 0;
static int file_descriptor_capacity = 100;

size_t 
file_size(int fdno)
{
	struct filedesc *fd = file_descriptors[fdno];
	size_t size = 0;
	struct block *block = fd->file->block_list;
	while (block != NULL) {
		size += block->occupied;
		block = block->next;
	}
	return size;
}

ufs_error_code
ufs_errno()
{
	return ufs_err_code;
}

struct file *
find_file_struct(const char *filename)
{
	struct file *file = file_list;
	while (file != NULL) {
		if (!strcmp(file->name, filename)) {
			return file;
		}
	}
	return NULL;
}

struct filedesc *
find_filedesc(struct file *file)
{
	struct filedesc *fd;
	for (size_t i = 0; i < file_descriptor_count; ++i) {
		fd = file_descriptors[i];
		if (fd->file == file) {
			return fd;
		}
	}
	return NULL;
}

int
ufs_open(const char *filename, int flags)
{
	struct file *file;
	struct filedesc *fd = malloc(sizeof(*fd));
	if (flags & UFS_CREATE) {
		file = alloc_file_struct(filename);
		fd->offset = 0;
		flags &= ~UFS_CREATE;
	} else {
		file = find_file_struct(filename);
		assert(file != NULL);
		if (!file) {
			ufs_err_code = UFS_ERR_NO_FILE;
			print_err_msg(ufs_err_code);
			return -1;
		}
		struct filedesc * ffd = find_filedesc(file);
		assert(ffd != NULL);
		if (!ffd) {
			ufs_err_code = UFS_ERR_NO_FD;
			print_err_msg(ufs_err_code);
			return -1;
		}
		fd->offset = ffd->offset; 
	}
	++file->refs;
	fd->file = file;

	int fdno = file_descriptor_count;
	file_descriptors[file_descriptor_count++] = fd;

	if (flags & UFS_READ_WRITE) {
		fd->mode = UFS_READ_WRITE;
	}
	else if (flags & UFS_READ_ONLY) {
		fd->mode = UFS_READ_ONLY;
	} 
	else if (flags & UFS_WRITE_ONLY) {
		fd->mode = UFS_WRITE_ONLY;
	}
	return fdno; 
}

ssize_t
ufs_write(int fdno, const char *buf, const size_t size)
{
	struct filedesc *fd = file_descriptors[fdno];
	if (fd->mode & UFS_WRITE_ONLY || fd->mode & UFS_READ_WRITE) {
		return write_data(fd->file, buf, size);
	} else {
		ufs_err_code = UFS_ERR_NO_PERMISSION;
		print_err_msg(ufs_err_code);
		return -1;
	}
}

ssize_t
ufs_read(int fdno, char *buf, size_t size)
{
	struct filedesc *fd = file_descriptors[fdno];
	
	if (fd->mode & UFS_READ_ONLY || fd->mode & UFS_READ_WRITE) {
		size_t block_count = size / BLOCK_SIZE;
		
		size_t total_read = 0;
		struct block *bl = fd->file->block_list;
		for (size_t block = 0; block < block_count ; ++block) {
			memcpy(buf, bl->memory, bl->occupied);
			total_read += bl->occupied;
			bl = bl->next;
			if (bl == NULL) {
				break;
			}
			buf += bl->occupied;
		}
		if ( size % BLOCK_SIZE != 0 || bl != NULL) {
			memcpy(buf, bl->memory, size % BLOCK_SIZE);
			total_read += size % BLOCK_SIZE;
		}
		return total_read;
	} else {
		ufs_err_code = UFS_ERR_NO_PERMISSION;
		print_err_msg(ufs_err_code);
		return -1;
	}
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

size_t
allocate_block(const char *const data, const size_t size,struct block **first_block, struct block **last_block)
{
	assert(data != NULL);
	assert(size != 0);
	char *ptr = (char *)data;
	size_t mem_block_cnt = 1;
	size_t mem_block_remained = size % BLOCK_SIZE;
	size_t total_blocks = mem_block_cnt;
	if (size > BLOCK_SIZE) {
		mem_block_cnt = size / BLOCK_SIZE; 
		mem_block_remained = size % BLOCK_SIZE;
		total_blocks = mem_block_remained > 0 ? (mem_block_cnt + 1) : mem_block_cnt;
	}
	char *memory = malloc(total_blocks * BLOCK_SIZE);
	assert(memory != NULL);
	if (!memory) {
		ufs_err_code = UFS_ERR_NO_MEM;
		print_err_msg(ufs_err_code);
		exit(1);
	}
	struct block *blocks = malloc(total_blocks * sizeof(struct block));
	memset(memory, 0, total_blocks * BLOCK_SIZE);
	memset(blocks, 0, total_blocks * sizeof(struct block));
	size_t total_written = 0;
	*first_block = blocks;
	struct block *bl;
	for (size_t b = 0; b < total_blocks; ++b) {
		bl = blocks;
		bl->memory = memory;
		struct block *cur;
		if (b != total_blocks - 1) {
			memcpy(bl->memory, ptr, BLOCK_SIZE);
			ptr += BLOCK_SIZE;
			bl->occupied = BLOCK_SIZE;
			total_written += BLOCK_SIZE;
			memory += BLOCK_SIZE;
			bl->next = ++blocks;
			cur = bl;
			*last_block = cur;
			bl = bl->next;
			bl->prev = cur;
		} else {
			memcpy(bl->memory, ptr, mem_block_remained);
			ptr += mem_block_remained;
			bl->occupied = mem_block_remained;
			total_written += mem_block_remained;
			cur = bl;
			*last_block = cur;
		}
	}
	return total_written;
}

struct file *
alloc_file_struct(const char *const filename)
{
	struct file *file = malloc(sizeof(*file));
	assert(file != NULL);
	if (!file) {
		ufs_err_code = UFS_ERR_NO_MEM;
		print_err_msg(ufs_err_code);
		exit(1);
	}
	size_t name_len = strlen(filename);
	char *name = malloc(name_len + 1);
	name[name_len] = 0;
	strncpy(name, filename, name_len);
	file->refs = 0;
	file->name = name;
	file->prev = file_list;
	if (file->prev != NULL) {
		file->prev->next = file;
	}
	file_list = file;
	return file;
}

size_t 
write_data(struct file *file, const char *data, const size_t size)
{
	size_t sz = size;
	if (sz > MAX_FILE_SIZE) {
		sz = MAX_FILE_SIZE;
	}
	size_t bytes_written = allocate_block(data, sz, &file->block_list, &file->last_block);
	return bytes_written;
}

size_t  
read_data(struct file *file, const char *buf, const size_t size)
{
}

int
main(int argc, const char **argv) 
{
	file_descriptors = malloc(sizeof( struct filedesc * ) * file_descriptor_capacity);
	// char *mem = malloc(1550);
	// mem[1549] = 0;
	// memset(mem, 'x', 1549);
	// char *mem2 = malloc(1550);
	// mem2[1549] = 0;
	// memset(mem2, 'y', 1549);

	// struct block *last_block;
	// const char *demo = "Hello world suka blyad ebanaya dev art";
	// struct block *b1 = allocate_block(demo, strlen(demo), &last_block);
	// struct file *file1 = allocate_file(mem, 1550);
	// struct file *file2 = allocate_file(mem2, 1550);
	// ufs_open(buf, 0);
	int fd = ufs_open("demo.txt", UFS_CREATE | UFS_READ_WRITE);
	ssize_t written = ufs_write(fd, "Hello, world from first file", 
		sizeof("Hello, world from first file"));
	size_t size = file_size(fd);
	char buf[1024] = { 0 };
	size_t read = ufs_read(fd, buf, 10);
	return 0;
}