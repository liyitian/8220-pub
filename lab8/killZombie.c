/*************************************************************************
    > File Name: killZombie.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 27 Mar 2016 03:00:54 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>

#define mywait(arg) syscall(330, arg)

int main(int argc, char* argv[])
{
	if(argc>1){
		int pid = atoi(argv[1]);
		int ret = mywait(pid);
		if(ret < 0){
			perror("mywait");
		}
	}
	return 0;
}
