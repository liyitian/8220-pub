/*************************************************************************
    > File Name: user.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 09 Mar 2016 05:37:41 PM EST
 ************************************************************************/

#include<stdio.h>
#include<errno.h>
#include<sys/syscall.h>
#define deepsleep() syscall(328)

int main()
{
	printf("goodnight, Irene\n");
	deepsleep();
	sleep(10);
	printf("oops ... wake up!\n");
}
