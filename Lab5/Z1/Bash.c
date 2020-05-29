#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main( void )
{
    int pid = fork();
    if ( pid == 0 )
    {
        execlp( "sudo", "sudo", "bash", NULL );
    }

    int status;

    // wait for bash to finish
    wait( &status );

    return 0;
}
