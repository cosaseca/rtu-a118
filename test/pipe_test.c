/*
 * pipe_test.c
 *
 *  Created on: Oct 14, 2014
 *      Author: ygz
 */


#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>
#include <fcntl.h>

int main( void )
{
    int filedes[2];
    char buf[80];
    pid_t pid;

    pipe( filedes );

//    fcntl(filedes[0], F_SETFL, O_NONBLOCK);
//    fcntl(filedes[1], F_SETFL, O_NONBLOCK);

    pid=fork();
    if (pid > 0)
    {
        printf( "This is in the father process,here write a string to the pipe.\n" );
        char s[] = "Hello world , this is write by pipe.\n";
        while(1) {
        	write( filedes[1], s, sizeof(s) );
        	sleep(2);
        }
        close( filedes[0] );
        close( filedes[1] );
    }
    else if(pid == 0)
    {
    	fd_set fdset;
    	fd_set rdset;
    	FD_ZERO(&fdset);
		FD_SET(filedes[0], &fdset);

		struct timeval s_timeval = {1, 0};
		int rc;
    	printf( "This is in the child process,here read a string from the pipe.\n" );
        while(1) {
        	rdset = fdset;
        	s_timeval.tv_sec = 1;
			s_timeval.tv_usec = 0;
			rc = select(filedes[0] + 1, &rdset, NULL, NULL, &s_timeval);
			switch(rc) {
				case -1:{
					perror("select\n");
					exit(1);
					break;
				}
				case 0:{
					printf("select rc %d\n", rc);
					break;
				}
				default:{
					if(FD_ISSET(filedes[0], &fdset)) {
						read( filedes[0], buf, sizeof(buf) );
						printf( "%s\n", buf );
					}
				}
			}
        }
        close( filedes[0] );
        close( filedes[1] );
    }

    waitpid( pid, NULL, 0 );

    return 0;
}


