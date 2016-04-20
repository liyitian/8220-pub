/* 
Here's the code ... crank up 8 of these in parallel and watch
the disk smoke.  Syntax is:

randread <start_dir> <seed> <file_prob.> <dir_factor>

Be sure that some of them start at "/"

Enjoy!
-RG
p.s. the code is largely due to Mike Westall, so please give him
credit if you're writing up experiments on it ...
*/

/* randread.c */

/* This function reads a unix directory tree... It can be */
/* used as a base for a file finder or space computer     */
/* or general command sweeper.                            */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
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

/* These constants control the workload generation */

static int     rseed;           /* Seed for generator              */
static float   fileprob;        /* Probability a file will be read */
static float   dirprob;         /* Probability a directory will be */
                                /* processed.                      */
static float   dirfactor;       /* Factor used to reduce dirprob   */
                                /* at each recursive call.         */

char databuf[512];

long get_ms(struct timespec time)
{
    return ((long) time.tv_sec * MSEC_PER_SEC) + (time.tv_nsec / 1000000L);
};

float unival()
{
   float fval;
   fval = (float)random() / RAND_MAX;
   return(fval);
}

/**/
/* Read a file in sequential hunks of 512 bytes */

readfile(
char *fname)
{
   FILE *input;
   int  blocks;
   int  amt;

   input = fopen(fname, "r");

   if (input != 0)
   {
       blocks = 0;
       do
       {
           amt = fread(databuf, 1, sizeof(databuf), input);
           blocks += 1;
       }   while(amt > 0);
       //printf("  %s %8d \n", fname, blocks);
       fclose(input);
   }
}

/**/
/* Recover the inode data for the current file or directory */

statfile(
char *fname,           /* Name of the file to stat..           */
struct stat *sbuf,     /* Address of the stat return buffer    */
char *cwdbuf)          /* Name of the active directory.        */
{

   int rc;
   char namebuf[512];          /* Fully qualified name buffer. */

   strcpy(namebuf, cwdbuf);
   strcat(namebuf, "/");
   strcat(namebuf, fname);

   rc = lstat(namebuf, sbuf);

}

/**/
/* Process all the files in the directory pointed to by DIRP */

procdir(
DIR *dirp,               /* Open directory handle.           */
char *cwdbuf)            /* Currently active directory       */
{
   struct dirent *dbuf;
   struct stat    sbuf;

/* Read a file or directory name */

   dbuf = readdir(dirp);

   while (dbuf != 0)
   {

   /* Get the inode data */

      statfile(dbuf->d_name, &sbuf, cwdbuf);
      //printf("%04x  %s \n", sbuf.st_mode, dbuf->d_name);
      fflush(stdout);

   /* Recursively process subdirectories.. This is the easy  */
   /* way to aviod recursively processing self or parent but */
   /* it misses any real directories starting with .         */

   /* more rmg mods to prevent hang */
      if (dbuf->d_name[0] != '.' && (S_ISDIR(sbuf.st_mode) ||
         S_ISREG(sbuf.st_mode)) )
      {

      /* The recursive call to nextdir will change the cwd  */
      /* chdir("..") can't be used to get back because of   */
      /* links... 					    */

         if (S_ISDIR(sbuf.st_mode))
         {
            nextdir(dbuf->d_name);    /* Process sub directory */
            chdir(cwdbuf);            /* Change back to current */
         }
         else if (S_ISREG(sbuf.st_mode))
         {
             if (unival() <= fileprob)
                readfile(dbuf->d_name);
         }
      }
      dbuf = readdir(dirp);
   }
}

/**/

/* Start a new directory */

nextdir(
char *dirname)
{
   DIR *dirp;               /* Directory handle */
   char cwdbuf[512];        /* Name buffer.     */

   if (unival() >= dirprob)
      return;

   dirprob *= dirfactor;
   dirp = opendir(dirname);

   if (dirp != 0)
   {
      chdir(dirname);
      getcwd(cwdbuf, sizeof(cwdbuf));
      //printf("\n---> Processing directory %s \n", cwdbuf);
      if(strcmp(cwdbuf,"/proc")) procdir(dirp, cwdbuf);
      closedir(dirp);
   /* chdir("..");   */
   }
   else
   {
      //printf("Failed to open directory %s\n", dirname);
      //printf("From directory %s \n", cwdbuf);
   }
   dirprob /= dirfactor;
}

main(
int   argc,
char  **argv)
{
   long wait, serv, reqs;
   rseed     = 1;
   fileprob  = 1.0;
   dirprob   = 1.0;
   dirfactor = 1.0;

    rseed = .3;
    fileprob = .6;
   //if (argc > 2)
   //   sscanf(argv[2], "%d", &rseed);
   //if (argc > 3)
    //  sscanf(argv[3], "%f", &fileprob);
   //if (argc > 4)
   //   sscanf(argv[4], "%f", &dirfactor);

   //fprintf(stderr, "Random reader processing directory %s \n", argv[1]);
   //fprintf(stderr, "Srand = %d.. Fileprob = %f.. Dirfactor = %f \n",
     //               rseed, fileprob, dirfactor);
   
   int threads[16];
   int my_pid;
   int i;
   init_things();
   for (i = 0; i < 16; i++)
   {
        my_pid = fork();
        threads[i] = my_pid;
        if (threads[i] == 0)
        {
		printf("Thread#: %d\n", i);
   		srandom(rseed);
   		nextdir("/");
   		get_summary(&wait, &serv, &reqs);
		printf("Thread died#: %d\n", i);
		return 0;
	}
   }
   pid_t pid;
   int status;
   struct req_stuff data[REQS_REQUESTED];
   long num_reqs;
   int started = 0;
   while (1)
   {
       for (i = 0; i < 16; i++)
       {
		pid = waitpid(threads[i], &status, WNOHANG);
		if (WIFEXITED(status))
		{ 
			my_pid = fork();
			if (my_pid == 0)
			{
				srandom(rseed);
				nextdir("/");
				get_summary(&wait, &serv, &reqs);
				printf("Thread died again#: %d\n", i);
				return 0;
			}			
		}	
       }
       get_things(&num_reqs);
       if (!started && num_reqs > 500)
       {    
	    started = 1;    
            init_things();
       }
       if (started && num_reqs > 19500)
       {
           end_things(&data);
           break;
       }
       //printf("NUM_REQS: %ld\n", num_reqs);
   }
   printf("%d, %ld, %ld, %ld\n", data[0].pid, get_ms(data[i].arrival), get_ms(data[i].serv_start), get_ms(data[i].end));
   return 0;
}
