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
		break;
	default:
		fprintf(stderr, "Error: Incorrect error number\n");
		break;
	}
}

struct file * alloc_file_struct(const char *const filename);
void deallocate_block(struct block **first_block);

size_t  write_data(struct file *file, const char *data, const size_t size);
size_t  read_data(struct filedesc *const fd, char *buf, 
	const size_t block_count, const size_t remained);

/** Global error code. Set from any function on any error. */
static ufs_error_code ufs_err_code = UFS_ERR_NO_ERR;

static struct file *file_list = NULL;

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
	if (!fd) {
		ufs_err_code = UFS_ERR_NO_FILE;
		print_err_msg(ufs_err_code);
		return -1;
	}
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
		if (file->name != NULL) {
			if (!strcmp(file->name, filename)) {
				return file;
			}
		}
		file = file->prev;
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
	file = find_file_struct(filename);
	if (flags & UFS_CREATE) {
		if (file != NULL) { // if we want to create file but it already exist
			free(fd);
			fd = NULL;
			ufs_err_code = UFS_ERR_NO_PERMISSION;
			print_err_msg(ufs_err_code);
			return -1;
		}
		file = alloc_file_struct(filename); 
		fd->offset = 0;
		flags &= ~UFS_CREATE;
	} else { // if we want just open the file (w/o create)
		// assert(file != NULL);
		if (!file) {
			free(fd);
			fd = NULL;
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

	int fdno = file_descriptor_count++;
	file_descriptors[fdno] = fd;

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
	if (!fd) {
		ufs_err_code = UFS_ERR_NO_FILE;
		print_err_msg(ufs_err_code);
		return -1;
	}
	if (fd->mode & UFS_WRITE_ONLY || fd->mode & UFS_READ_WRITE) {
		size_t bytes_written = write_data(fd->file, buf, size);
		fd->offset = bytes_written;
		return bytes_written;
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
	if (!fd) {
		ufs_err_code = UFS_ERR_NO_FILE;
		print_err_msg(ufs_err_code);
		return -1;
	}
	if (fd->mode & UFS_READ_ONLY || fd->mode & UFS_READ_WRITE) {
		size_t block_count = size / BLOCK_SIZE;
		size_t remained = size % BLOCK_SIZE;
		size_t bytes_read = read_data(fd, buf, block_count, remained);
		fd->offset = bytes_read;
		return bytes_read;
	} else {
		ufs_err_code = UFS_ERR_NO_PERMISSION;
		print_err_msg(ufs_err_code);
		return -1;
	}
}

void
deallocate_file(struct file **file)
{
	if ((*file)->block_list != NULL) {
		deallocate_block(&(*file)->block_list);
	}
	if ((*file)->name) {
		free(((char *)(*file)->name));
		(*file)->name = NULL;
	}
	free(*file);
	*file = NULL;
}

int
ufs_close(int fdno)
{
	struct filedesc *fd = file_descriptors[fdno];
	if (!fd) {
		ufs_err_code = UFS_ERR_NO_FILE;
		print_err_msg(ufs_err_code);
		return -1;
	}
	if (--fd->file->refs == 0) {
		deallocate_file(&(fd->file));
	}
	free(fd);
	fd = NULL;
	--file_descriptor_count;
	return 0; // SUCCESS
}

int
ufs_delete(const char *filename)
{
	struct file *file = find_file_struct(filename);
	if (file != NULL) {
		free((char *)file->name); // Allow to create a new file with the same name.
		file->name = NULL;
		return 0;
	}
	return -1;	
}

void 
deallocate_block(struct block **first_block) 
{
	assert(*first_block != NULL);
	free((*first_block)->memory);
	(*first_block)->memory = NULL;
	free(*first_block);
	*first_block = NULL;
}

size_t
allocate_block(const char *const data, const size_t size, 
	struct block **first_block, struct block **last_block)
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
	char *memory = 
		malloc(total_blocks * BLOCK_SIZE); 
	assert(memory != NULL);
	if (!memory) {
		ufs_err_code = UFS_ERR_NO_MEM;
		print_err_msg(ufs_err_code);
		exit(1);
	}
	struct block *blocks = 
		malloc(total_blocks * sizeof(struct block));
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
	file->next = NULL;
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
read_data(struct filedesc *const fd, char *buf, 
	const size_t block_count, const size_t remained)
{
	size_t total_read = 0;
	struct block *bl = fd->file->block_list;
	for ( size_t block = 0; block < block_count ; ++block ) {
		memcpy(buf, bl->memory, bl->occupied);
		total_read += bl->occupied;
		bl = bl->next;
		if (bl == NULL) {
			break;
		}
		buf += bl->occupied;
		fd->offset += bl->occupied;
	}
	if ( remained != 0 || bl != NULL ) {
		memcpy(buf, bl->memory, remained);
		total_read += remained;
		fd->offset += remained;
	}
	assert(fd->offset == total_read);
	return total_read;
}

// int
// main(int argc, const char **argv) 
// {
// 	file_descriptors = malloc(sizeof( struct filedesc * ) * file_descriptor_capacity);
// 	int fd = ufs_open("demo.txt", UFS_CREATE | UFS_READ_WRITE);
// 	int nfd = ufs_open("demo2.txt", UFS_CREATE | UFS_READ_WRITE);
// 	ssize_t written = ufs_write(fd, "Hello, world from first file", 
// 		sizeof("Hello, world from first file"));
// 	size_t size = file_size(fd);
// 	int fd2 = ufs_open("demo.txt", UFS_READ_WRITE | UFS_CREATE);
// 	int fd3 = ufs_open("demo.txt", UFS_READ_WRITE );
// 	int res = ufs_delete("demo.txt");
// 	int fd4 = ufs_open("demo.txt", UFS_READ_WRITE | UFS_CREATE);
// 	// ufs_close(fd2);
// 	ufs_close(fd3);
// 	ufs_close(fd);
// 	ufs_close(fd4);
// 	ufs_close(nfd);
// 	// char buf[1024] = { 0 };
// 	// size_t read = ufs_read(fd, buf, 10);

// 	free(file_descriptors);
// 	return 0;
// }