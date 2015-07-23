#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../sds.h"
#include "../test_help.h"

int main(void) {
	{
		struct sdshdr *sh;
		sds x = sds_new("foo"), y;

		test_cond("Create a string and obtain the length",
				sds_len(x) == 3 && memcmp(x,"foo\0",4) == 0)

			sds_free(x);
		x = sds_new_len("foo",2);
		test_cond("Create a string with specified length",
				sds_len(x) == 2 && memcmp(x,"fo\0",3) == 0)

			x = sds_cat(x,"bar");
		test_cond("Strings concatenation",
				sds_len(x) == 5 && memcmp(x,"fobar\0",6) == 0);

		x = sds_cpy(x,"a");
		test_cond("sds_cpy() against an originally longer string",
				sds_len(x) == 1 && memcmp(x,"a\0",2) == 0)

			x = sds_cpy(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
		test_cond("sds_cpy() against an originally shorter string",
				sds_len(x) == 33 &&
				memcmp(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0",33) == 0)

			sds_free(x);
		x = sds_cat_printf(sds_empty(),"%d",123);
		test_cond("sds_cat_printf() seems working in the base case",
				sds_len(x) == 3 && memcmp(x,"123\0",4) == 0)

			sds_free(x);
		x = sds_new("--");
		x = sds_cat_fmt(x, "Hello %s World %I,%I--", "Hi!", LLONG_MIN,LLONG_MAX);
		test_cond("sds_cat_fmt() seems working in the base case",
				sds_len(x) == 60 &&
				memcmp(x,"--Hello Hi! World -9223372036854775808,"
					"9223372036854775807--",60) == 0)

			sds_free(x);
		x = sds_new("--");
		x = sds_cat_fmt(x, "%u,%U--", UINT_MAX, ULLONG_MAX);
		test_cond("sds_cat_fmt() seems working with unsigned numbers",
				sds_len(x) == 35 &&
				memcmp(x,"--4294967295,18446744073709551615--",35) == 0)

			sds_free(x);
		x = sds_new("xxciaoyyy");
		sds_trim(x,"xy");
		test_cond("sds_trim() correctly trims characters",
				sds_len(x) == 4 && memcmp(x,"ciao\0",5) == 0)

			y = sds_dup(x);
		sds_range(y,1,1);
		test_cond("sds_range(...,1,1)",
				sds_len(y) == 1 && memcmp(y,"i\0",2) == 0)

			sds_free(y);
		y = sds_dup(x);
		sds_range(y,1,-1);
		test_cond("sds_range(...,1,-1)",
				sds_len(y) == 3 && memcmp(y,"iao\0",4) == 0)

			sds_free(y);
		y = sds_dup(x);
		sds_range(y,-2,-1);
		test_cond("sds_range(...,-2,-1)",
				sds_len(y) == 2 && memcmp(y,"ao\0",3) == 0)

			sds_free(y);
		y = sds_dup(x);
		sds_range(y,2,1);
		test_cond("sds_range(...,2,1)",
				sds_len(y) == 0 && memcmp(y,"\0",1) == 0)

			sds_free(y);
		y = sds_dup(x);
		sds_range(y,1,100);
		test_cond("sds_range(...,1,100)",
				sds_len(y) == 3 && memcmp(y,"iao\0",4) == 0)

			sds_free(y);
		y = sds_dup(x);
		sds_range(y,100,100);
		test_cond("sds_range(...,100,100)",
				sds_len(y) == 0 && memcmp(y,"\0",1) == 0)

			sds_free(y);
		sds_free(x);
		x = sds_new("foo");
		y = sds_new("foa");
		test_cond("sds_cmp(foo,foa)", sds_cmp(x,y) > 0)

			sds_free(y);
		sds_free(x);
		x = sds_new("bar");
		y = sds_new("bar");
		test_cond("sds_cmp(bar,bar)", sds_cmp(x,y) == 0)

			sds_free(y);
		sds_free(x);
		x = sds_new("aar");
		y = sds_new("bar");
		test_cond("sds_cmp(bar,bar)", sds_cmp(x,y) < 0)

			sds_free(y);
		sds_free(x);
		x = sds_new_len("\a\n\0foo\r",7);
		y = sds_cat_repr(sds_empty(),x,sds_len(x));
		test_cond("sds_cat_repr(...data...)",
				memcmp(y,"\"\\a\\n\\x00foo\\r\"",15) == 0)

		{
			int oldfree;

			sds_free(x);
			x = sds_new("0");
			sh = (void*) (x-(sizeof(struct sdshdr)));
			test_cond("sds_new() free/len buffers", sh->len == 1 && sh->free == 0);
			x = sds_make_room_for(x,1);
			sh = (void*) (x-(sizeof(struct sdshdr)));
			test_cond("sds_make_room_for()", sh->len == 1 && sh->free > 0);
			oldfree = sh->free;
			x[1] = '1';
			sds_incr_len(x,1);
			test_cond("sds_incr_len() -- content", x[0] == '0' && x[1] == '1');
			test_cond("sds_incr_len() -- len", sh->len == 2);
			test_cond("sds_incr_len() -- free", sh->free == oldfree-1);
		}
		sds_free(x);
		sds_free(y);
		x = sds_new_ull(1234235434985);
		printf("num sds is %s\n", x);
		sds_free(x);
	}
	test_report()
		return 0;
}
