#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/syscall.h>
#include <unistd.h>

//tyler
#define REQS_REQUESTED 19500
struct req_stuff
{
    int pid;
    struct timespec arrival;
    struct timespec serv_start;
    struct timespec end;
};

#define init_summary() syscall(330)
#define get_summary(wait, serv, reqs) syscall(331, wait, serv, reqs)
#define init_things() syscall(332)
#define get_things(req) syscall(333, req)
#define end_things(data) syscall(334, data)

#define MSEC_PER_SEC 1000

long get_ms(struct timespec time)
{
    return ((long) time.tv_sec * MSEC_PER_SEC) + (time.tv_nsec / 1000000L);
};

int main(void)
{
    int i;
    long reqs;
    struct req_stuff data[REQS_REQUESTED];
    int started = 0;
    int which = fork();
    if (which == 0)
    {
    	int status = system("./822_mptrace");
	return 0;
    }
    init_things();
    do
    {
        get_things(&reqs);
        if (!started && reqs > 500)
	{
		started = 1;
		init_things();
	}
    }
    while (reqs < REQS_REQUESTED);
    end_things(&data);
    for (i = 0; i < REQS_REQUESTED; i++)
    {
        fprintf(stderr, "%d, %ld, %ld, %ld\n", data[0].pid, get_ms(data[i].arrival), get_ms(data[i].serv_start), get_ms(data[i].end));
    }
    return 0;
}
