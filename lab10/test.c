/*************************************************************************
    > File Name: test.c
    > Author: yaolong yu
    > Mail: yaolony@g.clemson.edu 
    > Created Time: Wed 06 Apr 2016 07:58:11 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<sys/syscall.h>
#define sum_init syscall(332)
#define sum_print syscall(333)


int main(void)
{
	sum_init;
	sum_print;
	
	while(1){
		sleep(2);
		sum_print;
	}
	return 0;
}


