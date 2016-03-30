/*************************************************************************
    > File Name: wakeup.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sat 26 Mar 2016 01:07:33 AM EDT
 ************************************************************************/

#include<stdio.h>

#define deepwakeup(arg) syscall(329, arg)


int main(int argc, char* argv[])
{
	if(argc>1)
	{
		deepwakeup(atoi(argv[1]));	
		kill(atoi(argv[1]), 9);
	}
	else
	{
		printf("need you assign a target pid!\n");
	}
	return 0;
}
