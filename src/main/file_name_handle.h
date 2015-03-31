#include <stdlib.h>
#include <string.h>

#include "namespace_param.h"

const char file_dir_separator = '/';
/**
 * 校验文件路径
 */
int file_path_verify(const char *file_path) {
	return 1;
}

/**
 * 校验目录路径
 */
int dir_path_verify() {
	return 0;
}

void path_pre_handle(char *path) {
	int length = strlen(path);
	int tmp = length - 1;
	while (tmp >= 1 && *(path + tmp) == '/') {
		*(path + tmp--) = 0;
	}
}

/**
 * 将path分为目录名 + 文件名
 * @parent_dir_name 保存目录名称
 * @file_name 保存文件名称
 */
int parse_path(char *parent_dir_name, char *file_name, char *path, int len) {
	if (!file_path_verify(path))
		return illegal_path;
	path_pre_handle(path);
	char *tmp_path = (char*) malloc(sizeof(char) * len);
	strcpy(tmp_path, path);
	char *file_name_start = strrchr(tmp_path, '/');
	strcpy(file_name, file_name_start + 1);
	*file_name_start = 0;
	strcpy(parent_dir_name, tmp_path);
	free(tmp_path);
	return 0;
}
