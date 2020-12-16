#include "runtime.h"
#include <Sofa.h>
#include <stdio.h>
#include <dirent.h>


int main(int argc, char *argv[])
{
    
    RuntimeInit2(argc, argv);
    VFSClientInit();

    SFPrintf("App2 started\n");

    DIR *folder;

    folder = opendir("/");
    if(folder == NULL)
    {
        SFPrintf("Unable to read directory\n");
        return(1);
    }
    else
    {
        SFPrintf("Directory is opened!\n");
        struct dirent *entry;
        int files = 0;
        while( (entry=readdir(folder)) )
        {
            files++;
            SFPrintf("File %3d: %s\n",
                    files,
                    entry->d_name
                );
            if(files>10)
            {
                break;
            }
        }
        SFPrintf("found %i files\n", files);
        closedir(folder);
    }
    


    return 1;
}

