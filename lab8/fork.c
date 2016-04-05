/*************************************************************************
    > File Name: usr.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Thu 24 Mar 2016 03:38:12 PM EDT
 ************************************************************************/

#include<stdio.h>
#include<errno.h>
#include<sys/syscall.h>
#include<sys/signal.h>
#include<time.h>
#define smunch(pid, bits) syscall(331, pid, bits)

void son(int arg)
{
	printf("child get sig:%d\n", arg);
}


int main()
{
	int pid, ret;
	switch(pid = fork())
	{
		case 0:
				signal(SIGUSR1, son);
				signal(SIGCONT, son);
				signal(SIGURG, son);
				signal(SIGWINCH, son);
				while(1){
					printf("Child is playing\n");
				 	sleep(3);
				}
				break; 
		default:
				ret = smunch(pid, 138543616);
				while(1){
				 	//printf("parent is going to sleep\n");
					sleep(3);
					//ret = kill(pid, SIGUSR1);
					printf("smunch return:%d\n", ret);
					
				} 
	}
	return 0;

	
}
