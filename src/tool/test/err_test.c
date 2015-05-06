#include "../errinfo.h"
#include <stdio.h>
#include <errno.h>
int main()
{
	int x;
	err_ret("It's a test file");
	err_msg("just a message");
	errno = 3;
	err_ret("show me");
	err_ret("show me");
	return 0;
}
