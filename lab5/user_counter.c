/*************************************************************************
    > File Name: user_counter.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Wed 09 Mar 2016 05:51:55 PM EST
 ************************************************************************/

#include<stdio.h>
#include<errno.h>
#include<sys/syscall.h>
#include<sys/signal.h>
#include<time.h>

#define My_SysCall() syscall(325)
#define init(arg)		syscall(326,arg)
#define get(arg)		syscall(327,arg)
 
void son()
{
	get(SIGUSR1);
}

void test()
{
	printf("there\n");
}

int main()
{
	int pid, ret;
	switch(pid == fork()){
		case 0:{
			signal(SIGUSR1, son);
			signal(SIGWINCH, test);
			init(pid);
			while(1){

				printf("child is playing\n");
				sleep(1);
			}
			break;
		}
		default:{
		while(1){

		printf("parent is going to sleep!\n");
		sleep(5);
		printf("wake up, check on child\n\n");
		ret = kill(pid, SIGUSR1);
		printf("kill USR1 returned %d\n\n");
		ret = kill(pid, SIGSTOP);
		printf("kill SIGSTOP returned %d\n\n");
		ret = kill(pid, SIGCONT);
		printf("kill SIGCONT returned %d\n\n");
		ret = kill(pid, SIGWINCH);
		printf("kill SIGWINCH returned %d\n\n");
		}
		}
		
	}
	return 0;
}


