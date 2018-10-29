#include "FileServer.h"
#include <cpio/cpio.h>
#include <stdio.h>
#include <stdlib.h>

extern char _cpio_archive[];


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
