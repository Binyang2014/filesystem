#include <stdio.h>
#include <unistd.h>

int main()
{
	int ans = access("/home", F_OK);
	printf("ans is %d\n", ans);
}
