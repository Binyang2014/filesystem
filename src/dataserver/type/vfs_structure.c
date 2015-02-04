/**
 * vfs_structure.c
 * created on: 2015.2.2
 * author: Binyang
 */

#include "vfs_structure.h"
static void sb_set_block_bm(int block_num);
static void sb_clear_block_bm(int block_num);
static void sb_regist_block(int chunk_num, int block_num);
static void sb_logout_block(int chunk_num);
