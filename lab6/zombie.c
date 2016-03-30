/*************************************************************************
    > File Name: zombie.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Fri 25 Mar 2016 04:05:42 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>

int main()
{
	int pid;

	switch (pid = fork())
	{
		case 0:
			printf("try to kill pid %d\n", getpid());
			return 0;	
			break;
		default:
			printf("without killing pid: %d\n", getpid());
			while(1){
				sleep(10);
			}
	}

	return 0;

}
