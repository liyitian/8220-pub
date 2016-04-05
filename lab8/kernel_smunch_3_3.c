/*************************************************************************
    > File Name: smunch.c
    > Author: yaolong yu
    > Mail: yaolong@g.clemosn.edu 
    > Created Time: Tue 29 Mar 2016 08:14:18 PM EDT
 ************************************************************************/

#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<linux/wait.h>
#include<linux/sched.h>
#include<linux/signal.h>

SYSCALL_DEFINE2(smunch, int, pid, unsigned long, bit_pattern)
{
	sigset_t signal;
	unsigned long flags;
	int ret = 0;
	struct task_struct* p = NULL; 
	signal.sig[0] = bit_pattern;

	rcu_read_lock();
	p = find_task_by_vpid(pid);

	if(!p){
		printk(KERN_ALERT"Find task_struct failed\n");
		return -1;
	}
	if( (!thread_group_empty(p)) || (p->ptrace & PT_SEIZED)){
		printk(KERN_ALERT"process is Multi-thread or traced");
		return -1;
	}

	if(sigismember(&signal, SIGKILL)==1 && p->exit_state == EXIT_ZOMBIE){
		if(cmpxchg(&p->exit_state, EXIT_ZOMBIE, EXIT_DEAD) != EXIT_ZOMBIE)
			return -1;
		printk(KERN_ALERT"kill zombie\n");
		release_task(p);
		return 0;
	}
	if(lock_task_sighand(p,&flags)){
		p->pending.signal = signal;
		unlock_task_sighand(p, &flags);
	}
	ret = wake_up_process(p);

	printk(KERN_ALERT"in smunch\n");
	rcu_read_unlock();
	return 0;
}

