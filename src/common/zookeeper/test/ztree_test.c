#include <stdio.h>
#include "zookeeper.h"
#include "log.h"

int main()
{
	ztree_t *tree;
	zvalue_t *value;
	sds data, path, temp;

	log_init("", LOG_DEBUG);
	tree = create_ztree(1);
	//just add an ordinary node
	data = sds_new("zvalue");
	path = sds_new("/temp1");
	value = create_zvalue(data, PERSISTENT, 0);
	if(tree->op->add_znode(tree, path, value, NULL) == 0)
		printf("add znode successfully\n");
	destroy_zvalue(value);

	//try to find value
	value = NULL;
	value = tree->op->find_znode(tree, path);
	printf("the data is %s\n", value->data);
	destroy_zvalue(value);

	//add parent node
	value = NULL;
	data = sds_cpy(data, "/temp/tmp1");
	value = create_zvalue_parent(data, PERSISTENT, 0);
	path = sds_cpy(path, "/temp/tmp1");
	if(tree->op->add_znode(tree, path, value, NULL) == 0)
		printf("add znode successfully\n");
	destroy_zvalue(value);

	//add child to parent node
	value = NULL;
	data = sds_cpy(data, "value in /temp/tmp1:watch");
	value = create_zvalue(data, PERSISTENT, 0);
	path = sds_cpy(path, "/temp/tmp1:watch");
	if(tree->op->add_znode(tree, path, value, NULL) == 0)
		printf("add znode child successfully\n");
	destroy_zvalue(value);

	//add sub-parent
	value = NULL;
	data = sds_cpy(data, "This is a sub-parent");
	value = create_zvalue_parent(data, PERSISTENT, 0);
	path = sds_cpy(path, "/temp/tmp1:watch_p");
	if(tree->op->add_znode(tree, path, value, NULL) == 0)
		printf("add znode sub-parent successfully\n");
	destroy_zvalue(value);

	//add child to sub-parent
	value = NULL;
	data = sds_cpy(data, "This is children to sub-parent");
	value = create_zvalue_parent(data, PERSISTENT, 0);
	path = sds_cpy(path, "/temp/tmp1:watch_p/watch_sp");
	if(tree->op->add_znode(tree, path, value, NULL) == 0)
		printf("add znode  to sub-parent successfully\n");
	destroy_zvalue(value);

	//add other child to sub-sub-parent
	value = NULL;
	data = sds_cpy(data, "This is children to sub-sub-parent");
	value = create_zvalue(data, PERSISTENT_SQUENTIAL, 0);
	path = sds_cpy(path, "/temp/tmp1:watch_p/watch_sp/watch-");
	temp = sds_new_len("", 128);
	if(tree->op->add_znode(tree, path, value, temp) == 0)
		printf("add znode  to sub-sub-parent successfully, name is %s\n", temp);
	destroy_zvalue(value);

	//try to find value
	value = NULL;
	value = tree->op->find_znode(tree, temp);
	printf("the data is %s\n", value->data);
	destroy_zvalue(value);
	sds_free(temp);

	//delete znode
	path = sds_cpy(path, "/temp/tmp1:watch");
	tree->op->delete_znode(tree, path);
	value = tree->op->find_znode(tree, path);
	if(value == NULL)
		printf("delete value successfully\n");

	//delete znode in root
	path = sds_cpy(path, "/temp1");
	tree->op->delete_znode(tree, path);
	value = tree->op->find_znode(tree, path);
	if(value == NULL)
		printf("delete value successfully\n");

	//delete znode with children
	path = sds_cpy(path, "/temp/tmp1:watch_p");
	tree->op->delete_znode(tree, path);
	value = tree->op->find_znode(tree, path);
	if(value == NULL)
		printf("delete value successfully\n");
	path = sds_cpy(path, "/temp/tmp1:watch_p/watch_sp");
	value = tree->op->find_znode(tree, path);
	if(value == NULL)
		printf("Awesome!!!\n");

	sds_free(data);
	sds_free(path);
	destroy_ztree(tree);
	return 0;
}
