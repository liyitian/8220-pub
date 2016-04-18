#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>
#include <linux/blkdev.h>

extern long sum_of_services;
extern long sum_of_waits;
extern long num_requests_seen;

extern struct req_stuff stored_reqs[REQS_RECORDED];
extern int requests_seen;
extern bool serv_recording;

SYSCALL_DEFINE0(start_req_track)
{ 
    pr_alert("Init!");
    requests_seen = 0;
    serv_recording = 1;
    return 0;
}

SYSCALL_DEFINE1(end_req_track, struct req_stuff*, data)
{
    serv_recording = 0;
    copy_to_user(data, &stored_reqs, sizeof(struct req_stuff) * requests_seen);
    return 0;
}

