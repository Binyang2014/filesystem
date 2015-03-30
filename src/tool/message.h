
#ifndef SRC_TOOL_MESSAGE_H_
#define SRC_TOOL_MESSAGE_H_

#define CLIENT_MASTER_MESSAGE_SIZE 4096
#define CLIENT_MASTER_MESSAGE__CONTENT_SIZE 3072

const int create_file_code = 3001;
const int delete_file = 3002;

typedef struct{
	unsigned int file_size;
	char *file_name;
}create_file_structure;

#endif /* SRC_TOOL_MESSAGE_H_ */
