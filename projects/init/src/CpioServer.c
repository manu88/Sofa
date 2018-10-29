#include "FileServer.h"
#include <cpio/cpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>



static int CpioOpen(void* context, const char*pathname ,int flags);

extern char _cpio_archive[];

static const char cpioBaseName[] = "/cpio/";


static FileServerHandler _handler = { "/cpio/" ,   CpioOpen};

FileServerHandler* getCPIOServerHandler(void)
{
	return &_handler;
}



int CPIOServerInit()
{
	struct cpio_info info;

	if(cpio_info(_cpio_archive , &info) != 0)
	{
		return 0;
	}

	char **buf = malloc( info.file_count);

	for(int i=0;i<info.file_count;i++)
	{
		buf[i] = malloc(info.max_path_sz);
	}

	cpio_ls(_cpio_archive , buf, info.file_count);

	printf("Got %u files \n",info.file_count);

	for(int i=0;i<info.file_count;i++)
        {
		printf("File '%s'\n" , buf[i]);
	}



/* TEST*/


unsigned long fileSize = 0;
        void* dataContent = cpio_get_file(_cpio_archive , "hello.txt" , &fileSize);

	if( dataContent)
	{
		printf("Found Hello %lu bytes\n" , fileSize);
		printf("%.*s", fileSize, dataContent);
	}
	return 1;
}

static int CpioOpen(void* context, const char*pathname ,int flags)
{
	printf("CpioOpen '%s' , flags %i\n" , pathname , flags);

	unsigned long fileSize = 0;
	void* dataContent = cpio_get_file(_cpio_archive , pathname , &fileSize);

	if(dataContent == NULL)
	{
		return -ENOENT;
	}

	printf("CPIO File found : size %lu\n", fileSize);
	return -ENOSYS;
}
