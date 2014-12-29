#include <stdlib.h>
#include <string.h>
#include "../constant/namespaceparam.h"
#include "../constant/namespacestatus.h"

const char file_dir_separator = '/';

/**
 * Â·¾¶¸ñÊ½Ğ£Ñé
 * 1. ±ØĞëÒÔ'/'¿ªÍ·
 * 2.
 */
int file_path_verify(const char *file_path){
	return 1;
}

/**
 * ¶ÔÂ·¾¶½øĞĞÔ¤´¦Àí
 * ±ÈÈç: /foo/bar//// -> /foo/bar
 */
void path_pre_handle(char *path){
	int length = strlen(path);
	int tmp = length -1;
	while(tmp >= 1 && *(path + tmp) == '/'){
		*(path + tmp--) = 0;
	}
}

/**
 * å°†pathåˆ†ä¸ºç›®å½•å + æ–‡ä»¶å
 * @parent_dir_name ä¿å­˜ç›®å½•åç§°
 * @file_name ä¿å­˜æ–‡ä»¶åç§°
 */
int parse_path(char *parent_dir_name, char *file_name,  char *path, int len) {
	if(!file_path_verify(path))
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
