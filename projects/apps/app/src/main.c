#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <SysClient.h>

#include <errno.h>
#include <signal.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

int main( int argc , char* argv[])
{

    if (SysClientInit(argc , argv) != 0)
    {
        return 1;
    }



    printf("Client : Hello\n");


/*
    pid_t pid = getpid();
    pid_t parentPid = getppid();
    printf("Client PID is %i parent %i \n", pid, parentPid);

    int retNice = nice(-20);
    printf("Client nice returned %i errno %i\n",retNice, errno);
    errno = 0;

    int fd = open(NULL,0);
    assert(fd == -1);
    assert(errno == EFAULT);

    fd = open("/cpio/app" , O_RDWR);
    assert(errno == EPERM); // must fail 'cause cpio is only accessed with O_RDONLY

    errno = 0;

    fd = open("/cpio/hello.txt" , O_RDONLY);

    printf("Good open fd %i errno %i\n" ,fd, errno);

    errno = 0;

    char buf[4] = {0};

    ssize_t retRead  = read(fd , buf , 4);

    printf("App : read '%s'  %lu\n", buf , retRead);

    retRead  = read(fd , buf , 4);

    printf("App : read '%s'  %lu\n", buf , retRead);


    retRead  = read(fd , buf , 4);

    printf("App : read '%s'  %lu\n", buf , retRead);

    retRead  = read(fd , buf , 4);

    printf("App : read '%s'  %lu\n", buf , retRead);


    retRead  = read(fd , buf , 4);

    printf("App : read '%s'  %lu\n", buf , retRead);


    lseek( fd , 0,SEEK_SET);

	retRead  = read(fd , buf , 4);

    printf("App : AFTER SEEK read '%s'  %lu\n", buf , retRead);



    int fd2 = open("/dev/console" , O_RDONLY);

    printf("console open %i err %i\n" , fd2 , errno);
    errno  = 0;

    if(fd2 >= 0 )
    {
	const char b[] = "Hello world!";
	write(fd2 , b ,strlen(b));
    }
  */ 
//    kill(1 , SIGCONT);
    
    //while(1)
    {
    	int ret = usleep(1000*4000);
	assert(ret == 0);
	assert(errno == 0);
//        printf("Client %i did wait\n" , pid);
    }


    printf("Client After sleep\n");

    return 0;
}

