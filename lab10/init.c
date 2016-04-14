/*************************************************************************
    > File Name: test.c
    > Author: yaolong yu
    > Mail: yaolony@g.clemson.edu 
    > Created Time: Wed 06 Apr 2016 07:58:11 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<sys/syscall.h>
#define sum_init syscall(327)


int main(void)
{
	sum_init;
	return 0;
}


