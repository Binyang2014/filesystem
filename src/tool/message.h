
#ifndef SRC_TOOL_MESSAGE_H_
#define SRC_TOOL_MESSAGE_H_
/*****************************************************************************************/
/**********客户端 master间指令消息大小********/
#define CLIENT_MASTER_MESSAGE_SIZE 4096
/**********客户端 master消息体大小***********/
#define CLIENT_MASTER_MESSAGE__CONTENT_SIZE 3072



/*******指令操作码******/
const unsigned short create_file_code = 3001;
const int delete_file = 3002;



/*******TAG分类********/
#define CLIENT_INSTRCTION_MESSAGE 400
#define CLIENT_INSTRUCTION_ANS_MESSAGE 401

typedef struct{
	unsigned short instruction_code;
	long file_size;
	char file_name[CLIENT_MASTER_MESSAGE__CONTENT_SIZE];
}create_file_structure;

typedef struct{
	unsigned short ans_code;
	long bolck_nums;
};

#endif /* SRC_TOOL_MESSAGE_H_ */
