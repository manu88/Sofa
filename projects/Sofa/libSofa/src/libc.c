#include <bits/syscall.h>
#include <stdarg.h>
#include <sys/uio.h> // iovec
#include <sys/types.h>
#include <files.h>
#include <errno.h>


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
    SFPrintf("sf_open %s\n", pathname);
    return VFSOpen(pathname, flags);
//    return 0;
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
    case __NR_mmap:
        ret = sf_mmap(al);
        break;
    case __NR_munmap:
        ret = sf_munmap(al);
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