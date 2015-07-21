/**
 * This test file is to make true that vfs system works well
 *
 */
#include "../vfs_structure.h"
#include "../../../common/zmalloc.h"
#include "../../../common/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>

static uint64_t chunks_arr[10] = {0x345, 0xfff, 0x123456, 0x19203454, 0x12343, 0x12438959, 0x11111111, 0x2222222222222222,
		0x1243576, 0xff32};
static char write_buff[4200] = {0};
static char read_buff[4200] = {0};

void write_file(dataserver_file_t* d_sb, const char* file_path, off_t file_len);
void read_and_write_to_disk(dataserver_file_t* d_sb, const char* copy_path, off_t file_len);
void delete_file(dataserver_file_t* d_sb);

void s_hash_test(dataserver_sb_t* d_sb)
{
	uint32_t block_num = 255, hash_num;
	size_t chunk_num = 12;
	int i;

	uint32_t blocks_arr[10] = {12, 6543, 432657, 34, 1234, 4356, 1904, 8192, 7935, 23876};
	uint32_t blocks_arr_t[10];
	uint32_t* ans;

	//I will test if hash table works well
	hash_num = d_sb->s_op->alloc_a_block(d_sb, chunk_num, block_num);
	printf("hash number is %u\n", hash_num);
	hash_num = d_sb->s_op->find_a_block_num(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_sb->s_op->free_a_block(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_sb->s_op->find_a_block_num(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);

	d_sb->s_op->alloc_blocks(d_sb, 10, chunks_arr, blocks_arr);
	d_sb->s_op->find_serials_blocks(d_sb, 10, chunks_arr, blocks_arr_t);
	for(i = 0; i < 10; i++)
		printf("%u\t", blocks_arr_t[i]);
	printf("\n");

	d_sb->s_op->free_blocks(d_sb, 10, chunks_arr);
	ans = d_sb->s_op->find_serials_blocks(d_sb, 10, chunks_arr, blocks_arr_t);
	if(ans != NULL)
	{
		for(i = 0; i < 10; i++)
			printf("%u\t", blocks_arr_t[i]);
		printf("\n");
	}
}

void vfs_basic_read_write_test(dataserver_file_t* d_file)
{
	int i, arr_len;
	uint32_t offset;
	arr_len = d_file->arr_len;
	for(i = 0; i < arr_len; i++)
		d_file->f_chunks_arr[i] = chunks_arr[i];

	printf("write to write buffer\n");
	for(i = 0; i < 10; i++)
	{
		write_buff[i] = '0' + i;
		printf("%c ", write_buff[i]);
	}
	memcpy(write_buff + BLOCK_SIZE - 1, write_buff, 10);
	printf("\n");

	offset = 2;

	//write to one block with offset
	printf("Test read and write by writing to one block with offset\n");
	d_file->f_op->vfs_write(d_file, write_buff, 7, offset);
	d_file->f_op->vfs_read(d_file, read_buff, 7, offset);
	printf("read from blocks\n");
	for(i = 0; i < 10; i++)
	{
		printf("%c ", read_buff[i]);
	}
	memset(read_buff, 0 ,sizeof(read_buff));
	printf("\n-----------------------------------------------------------------\n");

	printf("Test read and write by writing across two blocks\n");
	d_file->f_op->vfs_write(d_file, write_buff, 7, BLOCK_SIZE -2);
	d_file->f_op->vfs_read(d_file, read_buff, 7, BLOCK_SIZE -2);
	for(i = 0; i < 7; i++)
	{
		printf("%c ", read_buff[i]);
	}
	memset(read_buff, 0 ,sizeof(read_buff));
	printf("\n------------------------------------------------------------------\n");

	printf("Test read and write by writing to many blocks with offset\n");
	d_file->f_op->vfs_write(d_file, write_buff, 5+BLOCK_SIZE, BLOCK_SIZE -2);
	d_file->f_op->vfs_read(d_file, read_buff, 5+BLOCK_SIZE, BLOCK_SIZE -2);
	for(i = 0; i < 10; i++)
	{
		printf("%c ", read_buff[i]);
	}
	for(i = 0; i < 10; i++)
	{
		printf("%c ", (read_buff + BLOCK_SIZE - 1)[i]);
	}
	printf("\nfinish basic read write test!!!\n");
}

int main()
{
	dataserver_sb_t* d_superblock;
	dataserver_file_t* d_file;
	vfs_hashtable_t* f_arr;
	int f_arr_len;

	log_init("", LOG_DEBUG);
	d_superblock = vfs_init(MIDDLE, 1);
	d_superblock->s_op->print_sb_imf(d_superblock);
	s_hash_test(d_superblock);
	printf("------------------------------------------------\n\n");

	printf("-------------test about file operations------------\n");
	//init structure of data server file
	d_file = (dataserver_file_t* )zmalloc(sizeof(dataserver_file_t));
	f_arr_len = 8;

	f_arr = init_hashtable(f_arr_len);
	d_file = init_vfs_file(d_superblock, d_file, f_arr, VFS_WRITE);

	vfs_basic_read_write_test(d_file);
	zfree(d_file);
	destroy_hashtable(f_arr);
	vfs_destroy(d_superblock);

	printf("---------------------------------------------\n\n");
	printf("-----------------large file test----------------\n");
	{
		struct stat stat_buf;
		const char* file_name = "/home/binyang/Videos/Game.of.Thrones.S05E00.A.Day.in.the.Life.720p.HDTV.x264-BATV.mkv";
		const char* copy_path = "/home/binyang/Documents/Game.of.Thrones.S05E00.A.Day.in.the.Life.720p.HDTV.x264-BATV.mkv";
		off_t file_len = 0;
		int chunks_count = 0, rest_counts = 0, i = 1;

		d_superblock = vfs_init(LARGE, 1);
		d_superblock->s_op->print_sb_imf(d_superblock);
		stat(file_name, &stat_buf);
		file_len = stat_buf.st_size;
		chunks_count = file_len / BLOCK_SIZE;
		rest_counts = file_len % BLOCK_SIZE;

		f_arr_len = rest_counts == 0 ? chunks_count : chunks_count + 1;
		d_file = (dataserver_file_t *)zmalloc(sizeof(dataserver_file_t));
		f_arr = init_hashtable(f_arr_len);

		//write same file twice to see if it can work well
		while(i > 0)
		{
			printf("write large file to memory file system.\n");
			init_vfs_file(d_superblock, d_file, f_arr, VFS_WRITE);
			write_file(d_file, file_name, file_len);
			d_superblock->s_op->print_sb_imf(d_superblock);

			printf("read file in memory file system and copy to another place\n");
			init_vfs_file(d_superblock, d_file, f_arr, VFS_READ);
			read_and_write_to_disk(d_file, copy_path, file_len);
			d_superblock->s_block->s_last_write_time = time(NULL);
			d_superblock->s_op->print_sb_imf(d_superblock);

			printf("delete file in memory file system\n");
			init_vfs_file(d_superblock, d_file, f_arr, VFS_DELETE);
			delete_file(d_file);
			d_superblock->s_op->print_sb_imf(d_superblock);

			if(init_vfs_file(d_superblock, d_file, f_arr, VFS_READ) == NULL)
				printf("delete file success\n");
			else
				printf("wrong when removing file from memory file system\n");

			i--;
		}
		zfree(d_file);
		destroy_hashtable(f_arr);
		vfs_destroy(d_superblock);
	}
	return 0;
}

void write_file(dataserver_file_t* d_file, const char* file_path, off_t file_len)
{
	FILE* fp;
	int i, rest_bytes;
	char data[BLOCK_SIZE];
	off_t write_len = 0;

	fp = fopen(file_path, "rb");
	for(i = 0; i < d_file->arr_len; i++)
		d_file->f_chunks_arr[i] = i;

	printf("begin write large file to data server\n");

	for(i = 0; i < d_file->arr_len - 1; i++)
	{
		fread(data, sizeof(char), BLOCK_SIZE, fp);
		write_len += d_file->f_op->vfs_write(d_file, data, BLOCK_SIZE, BLOCK_SIZE * i);
	}
	rest_bytes = file_len % BLOCK_SIZE;
	if(rest_bytes == 0)
		rest_bytes = BLOCK_SIZE;

	fread(data, sizeof(char), rest_bytes, fp);
	write_len += d_file->f_op->vfs_write(d_file, data, rest_bytes, BLOCK_SIZE * (d_file->arr_len - 1));
	fclose(fp);
	printf("--------------------------------------------------\n");
	printf("finish large file writing the file len is %llu and write file len is %llu\n", (unsigned long long)file_len, (unsigned long long)write_len);
}

void read_and_write_to_disk(dataserver_file_t* d_file, const char* copy_path, off_t file_len)
{
	FILE* fp;
	int i, rest_bytes;
	char data[BLOCK_SIZE];

	fp = fopen(copy_path, "wb");

	for(i = 0; i < d_file->arr_len - 1; i++)
	{
		d_file->f_op->vfs_read(d_file, data, BLOCK_SIZE, i * BLOCK_SIZE);
		fwrite(data, sizeof(char), BLOCK_SIZE, fp);
	}
	rest_bytes = file_len % BLOCK_SIZE;
	if(rest_bytes == 0)
		rest_bytes = BLOCK_SIZE;

	d_file->f_op->vfs_read(d_file, data, rest_bytes, BLOCK_SIZE * (d_file->arr_len - 1));
	fwrite(data, sizeof(char), rest_bytes, fp);
	fclose(fp);
}

void delete_file(dataserver_file_t* d_file)
{
	d_file->f_op->vfs_remove(d_file);
	printf("--------------------------------------------------\n");
	printf("finish large file removing\n"); 
}
