#include <bits/syscall.h>
#include <stdarg.h>
#include <sys/uio.h> // iovec
#include <sys/types.h>
#include <files.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

extern void *__sysinfo;


static long sf_close(va_list ap)
{
    int fd = va_arg(ap, int);

    return VFSClose(fd);
}

static long sf_write(va_list ap)
{
    int fd = va_arg(ap, int);
    const void *buf = va_arg(ap, void *);
    size_t bufSize = va_arg(ap, size_t);

    return VFSWrite(fd, buf, bufSize);
}
static long sf_read(va_list ap)
{
    int fd = va_arg(ap, int);
    char *buf = va_arg(ap, char *);
    int bufSize = va_arg(ap, int);

    return VFSRead(fd, buf, bufSize);
}
static long sf_open(va_list ap)
{
    const char *pathname = va_arg(ap, const char *);
    int flags = va_arg(ap, int);
    mode_t mode = va_arg(ap, mode_t);
    return VFSOpen(pathname, flags);
//    return 0;
}

static int c = 4;
static int numSent = 0;

static long sf_getdents64(va_list ap)
{
    unsigned int fd = va_arg(ap, int);
    struct dirent *dirp = va_arg(ap, struct dirent *);
    unsigned int count = va_arg(ap, unsigned int);

    return VFSRead(fd, dirp, count);
    if(numSent >= c)
    {
        return 0;
    }
    size_t numDirentPerBuff = count / sizeof(struct dirent);
    size_t numOfDirents = numDirentPerBuff > c? c:numDirentPerBuff;

    size_t nextOff = 0;
    size_t acc = 0;
    for(size_t i=0;i<numOfDirents;i++)
    {
        struct dirent *d = dirp + i;

        snprintf(d->d_name, 256, "File%i", i);
        acc += sizeof(struct dirent);
        d->d_off = acc;
        d->d_type = DT_DIR;
        d->d_reclen = sizeof(struct dirent);

        
    }
    numSent += numOfDirents;
    return acc;


    return 0;
    //On success, the number of bytes read is returned.
    //On end of directory, 0 is returned. On error, -1 is returned, 
    //and errno is set appropriately. 
}
static long sf_munmap(va_list ap)
{
    void *addr = va_arg(ap, void*);
    size_t length = va_arg(ap, size_t);
    return SFMunmap(addr, length);
}

static long sf_mmap(va_list ap)
{
    void *addr = va_arg(ap, void*);
    size_t length = va_arg(ap, size_t);
    int prot = va_arg(ap, int);
    int flags = va_arg(ap, int);
    int fd = va_arg(ap, int);
    off_t offset = va_arg(ap, off_t);
    return SFMmap(addr, length, prot, flags, fd, offset);
// mmap call with addr=0 length=4096 prot=3 flags=0x22 fd=-1 offset=0
//PROT_NONE
//PROT_EXEC
//PROT_READ
//PROT_WRITE
//MAP_SHARED

}


static long sf_fcntl(va_list ap)
{
    int fd = va_arg(ap, int);
    int cmd = va_arg(ap, int);
    if(cmd == F_SETFD)
    {
        return 0;
    } 
    SFPrintf("Not implemented: fcntl for fd=%i cmd=%u\n", fd, cmd);
    return -1;
}
static long sf_ioctl(va_list ap)
{
    int fd = va_arg(ap, int);
    unsigned long request = va_arg(ap, unsigned long);
    SFPrintf("Not implemented: ioctl for fd=%i request=%u\n", fd, request);
    return 0;
}

static long sf_writev(va_list ap)
{
    int  fildes = va_arg(ap, int);
    struct iovec *iov = va_arg(ap, struct iovec *);
    int iovcnt = va_arg(ap, int);

    ssize_t ret = 0;


    for (int i = 0; i < iovcnt; i++) 
    {
        char * base = (char *)iov[i].iov_base;
        SFPrintf("%s", base);
        VFSWrite(fildes, iov[i].iov_base, iov[i].iov_len);
        ret += iov[i].iov_len;
        /*
        for (int j = 0; j < iov[i].iov_len; j++) 
        {
            
            //seL4_DebugPutChar(base[j]);

            ret++;
        }
        */
    }
    //SFPrintf("writeev fd=%i size=%zi\n", fildes, ret);

    return ret;
}


long sofa_vsyscall(long sysnum, ...)
{
    va_list al;
    va_start(al, sysnum);
    long ret = -1;
    switch (sysnum)
    {
    case __NR_open:
        ret =  sf_open(al);
        break;
    case __NR_close:
        ret =  sf_close(al);
        break;
    case __NR_write:
        ret = sf_write(al);
        break;
    case __NR_read:
        ret = sf_read(al);
        break;
    case __NR_writev:
        ret = sf_writev(al);
        break;
    case __NR_brk:
        ret = -ENOMEM;
        break;
    case __NR_ioctl:
        ret = sf_ioctl(al);
        break;
    case __NR_fcntl:
        ret = sf_fcntl(al);
        break;
    case __NR_mmap:
        ret = sf_mmap(al);
        break;
    case __NR_munmap:
        ret = sf_munmap(al);
        break;
    case __NR_getdents64:
        ret = sf_getdents64(al);
        break;
    default:
    SFPrintf("Unknown syscall %zi\n", sysnum);
        break;
    }
    
    va_end(al);
    return ret;
}


void initMuslSysCalls(void)
{
    __sysinfo = sofa_vsyscall;

}