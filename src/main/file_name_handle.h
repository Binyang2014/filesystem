#include <stdlib.h>
#include "../constant/namespaceparam.h"

const char * legal_file_path = "";
const char file_dir_separator = '/';
/**
 * 将path分为目录名 + 文件名
 * @parent_dir_name 保存目录名称
 * @file_name 保存文件名称
 */
int parse_path(char *parent_dir_name, char *file_name, const char *path)
{
	char *tmp_path = (char*)malloc(sizeof(char) * file_name_max_length);
	strcpy(tmp_path, path);
	char *file_name_start = strrchr(tmp_path, file_dir_separator) + 1;
	strcpy(file_name, file_name_start);
	*file_name_start = 0;
	strcpy(parent_dir_name, tmp_path);
	free(tmp_path);
}
