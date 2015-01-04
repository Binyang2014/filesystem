#ifndef _FILESYSTM_CONSTANT_NAMESPACEPARAM_H_
#define _FILESYSTM_CONSTANT_NAMESPACEPARAM_H_



//文件名长度
const int file_name_max_length = 256;
//父目录映射长度
const int parent_dir_hash_length = 10;
//子目录影射长度
const int child_dir_hash_legth = 1024;



//文件名不合法
const int illegal_file_name = 1000;
//文件名已存在
const int file_exists = 1001;
//目录名不合法
const int illegal_dir_name = 1002;
//目录已经存在
const int dir_exists = 1003;
//目录不存在
const int dir_not_exists = 1004;
//路径非法
const int illegal_path = 1005;
//创建成功
const int create_success = 2000;
#endif
