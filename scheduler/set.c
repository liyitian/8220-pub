#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define init_summary() syscall(330)
#define get_summary(wait, serv, reqs) syscall(331, wait, serv, reqs)
#define init_things() syscall(332)

int main(void)
{
   init_things();
}
