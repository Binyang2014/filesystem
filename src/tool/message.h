
#ifndef SRC_TOOL_MESSAGE_H_
#define SRC_TOOL_MESSAGE_H_

#define CLIENT_MASTER_MESSAGE_SIZE 4096
#define CLIENT_MASTER_MESSAGE__CONTENT_SIZE 3072

/*******指令操作码******/
const unsigned short create_file_code = 3001;
const int delete_file = 3002;

/*******TAG分类********/
#define CLIENT_INSTRCTION_MESSAGE 400

typedef struct{
	unsigned short instruction_code;
	long file_size;
	char file_name[CLIENT_MASTER_MESSAGE__CONTENT_SIZE];
}create_file_structure;

#endif /* SRC_TOOL_MESSAGE_H_ */
