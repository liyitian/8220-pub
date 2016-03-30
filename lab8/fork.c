/*************************************************************************
    > File Name: fork.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sat 26 Mar 2016 01:58:32 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
int main()
{
	pid_t pid;
	switch(pid = fork()){
		case 0:
			printf("I am son\n");
			return 0;
			break;
		default:
			wait(NULL);
			perror("wait\n");
	}
	return 0;

}
