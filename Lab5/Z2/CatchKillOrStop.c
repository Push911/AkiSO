#include <stdio.h>
#include <signal.h>

void catchKill(int signal)
{
    printf("Sucessfully caught the kill signal\n");
}
void catchStop(int signal)
{
    printf("Sucessfully caught the kill signal\n");
}

int main(int argc,char **argv)
{
	int n;
   	printf( "Enter 1 to test kill signal \n");
	printf( "Enter 2 to test stop signal \n");
   	scanf("%d", &n);
	switch(n)
	{
		case 1:
		{
			if(signal(SIGKILL, catchKill)==SIG_ERR)
    			{
        			printf("Cannot catch kill signal\n");
    			}	
			break;
		}
		case 2:
		{
			if(signal(SIGSTOP, catchStop)==SIG_ERR)
    			{
        			printf("Cannot catch stop signal\n");
    			}
			break;	
		}
		default:
			printf("Enter proper value");
	}
    	return 0;
}
