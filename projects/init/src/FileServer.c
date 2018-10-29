#include "FileServer.h"
#include <cpio/cpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


extern char _cpio_archive[];

static const char cpioBaseName[] = "/cpio/";


static int prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int FileServerInit()
{
	int error = 0;

	struct cpio_info info;

	error = cpio_info( _cpio_archive , &info);

	if( error != 0)
	{
		return 0;
	}


	printf("Init : Cpio has %i files , max path %i\n", info.file_count , info.max_path_sz);

	char ** lsBuf = malloc( info.file_count);

	for(int i= 0;i<info.file_count;++i)
	{
		lsBuf[i] = malloc(info.max_path_sz);
	}

	cpio_ls( _cpio_archive , lsBuf , info.file_count);


	for(int i= 0;i<info.file_count;++i)
        {

		printf("File '%s'\n", lsBuf[i]);
	}

/*	for(int i= 0;i<info.file_count;++i)
        {
		free(lsBuf[i] );

	}
*/
//	free(lsBuf);
	return 1;
}

int FileServerOpen(InitContext* context , const char*pathname , int flags)
{
	assert(context);
	assert(pathname);



	if (prefix(cpioBaseName , pathname) )
	{
		printf("FileServer : got a CPIO request '%s'\n", (pathname+strlen(cpioBaseName)));
	}
	else
	{
		printf("FileServer : Got handle_open '%s' %i\n" , pathname,flags);
	}
	return -ENOSYS;
}
