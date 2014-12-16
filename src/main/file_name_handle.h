#include <stdlib.h>
#include "../constant/namespaceparam.h"

const char * legal_file_path = "";
const char file_dir_separator = '/';
/**
 * ��path��ΪĿ¼�� + �ļ���
 * @parent_dir_name ����Ŀ¼����
 * @file_name �����ļ�����
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
