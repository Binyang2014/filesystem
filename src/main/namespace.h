#ifndef NAMESPACE
#define NAMESPACE
/**
 * 创建文件
 * 
 */
extern int create_file(char *name, int lenght, int size);

//创建目录
//
extern int create_dir(char *name, int length);


/**
 *
 *
 */
extern int rename_file(char *name, int length);

/**
 *
 *
 */
extern int rename_dir(char *name, int length);


/**
 *
 *
 */
extern int del_file(char *name);

/**
 *
 *
 */
extern int del_dir(char *name);

#endif
