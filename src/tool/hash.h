// BKDR Hash Function
unsigned int BKDRHash(char *str, int length)
{
	unsigned int seed = 131;// 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;
	while (*str)
	{
		hash = hash*seed + (*str++);
	}
	return(hash % length);
}
