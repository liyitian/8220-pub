/*************************************************************************
    > File Name: user.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 17 Feb 2016 06:01:04 PM EST
 ************************************************************************/

#include<stdio.h>
#include<errno.h>
#include<sys/syscall.h>

#define the_goob(arg) syscall(325,arg)

int main()
{
	int ret;
	ret = the_goob(5);
	fprintf(stderr, "syscall return %d\n", ret);
	return 0;
}
