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
