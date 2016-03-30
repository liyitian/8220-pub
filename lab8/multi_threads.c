/*************************************************************************
    > File Name: multi_threads.c
    > Author: yaolong yu
    > Mail: yaolony@g.clemson.edu 
    > Created Time: Wed 30 Mar 2016 01:55:46 AM EDT
 ************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

pthread_t ntid;

void print_ids(const char* s)
{
	pid_t pid;
	pthread_t tid;
	pid = getpid();
	tid = pthread_self();
	printf("%s pid %u tid %u (0x%x)\n", s, (unsigned int)pid, (unsigned int)tid, (unsigned int)tid);
}

void *thr_fn(void *arg)
{
	print_ids(arg);
	while(1)
	{
		sleep(10);
	}
	return NULL;
}

int main(void)
{
	int err;
	err = pthread_create(&ntid, NULL, thr_fn, "new thread: ");
	if(err != 0){
		fprintf(stderr, "thread creat error %s\n", strerror(err));
		exit(1);
	}
	print_ids("main thread: ");
	while(1){
		sleep(10);
	}
	return 0;
}

