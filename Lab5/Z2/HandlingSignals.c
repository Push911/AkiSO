//ctrl+\ = SIGQUIT; ctrl+z = SIGTSTP; ctrl+c =SIGINT
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

void sighandler(int);

int main ()
{
    for(int i=1; i<=64; i++)
    {
        signal(i, sighandler);
    }
    while(1)
    {
        //printf("Going to sleep for a second...\n");
        sleep(1);
    }
    return 0;
}

void sighandler(int signum)
{
    printf("Caught signal %d, coming out...\n", signum);
    exit(1);
}
