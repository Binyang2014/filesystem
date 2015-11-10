/*
 * test_help.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_COMMON_TEST_HELP_H_
#define SRC_COMMON_TEST_HELP_H_

int __failed_tests = 0;
int __test_num = 0;

#define test_cond(descr,_c) do { \
    __test_num++; printf("%d - %s: ", __test_num, descr); \
	    if(_c) printf("PASSED\n"); else {printf("FAILED\n"); __failed_tests++;} \
} while(0);

#define test_report() do { \
    printf("%d tests, %d passed, %d failed\n", __test_num, \
			                    __test_num-__failed_tests, __failed_tests); \
	    if (__failed_tests) { \
		        printf("=== WARNING === We have failed tests here...\n"); \
			        exit(1); \
			    } \
} while(0);

#endif /* SRC_COMMON_TEST_HELP_H_ */
