/*************************************************************************
    > File Name: smunch.c
    > Author: yaolong yu
    > Mail: yaolony@g.clemson.edu 
    > Created Time: Tue 29 Mar 2016 10:50:37 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/types.h>
#define smunch(pid, bit_pattern) syscall(331, pid, bit_pattern) 

void sighandler(int arg)
{
	printf("sighandler arg:%d\n", arg);
}

int main(int argc, char *argv[])
{
	int ret;
	if(argc<2){
		printf("need pid!\n");
		return 1;
	}
	
	ret = smunch(atoi(argv[1]), 256);
	printf("smunch return: %d\n", ret);
	return 0;
}
