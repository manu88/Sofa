/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bits/syscall.h>
#include <stdarg.h>
#include <sys/uio.h> // iovec
#include <sys/types.h>
#include <sys/ioctl.h>
#include <files.h>
#include <net.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <proc.h>

extern void *__sysinfo;


static long sf_close(va_list ap)
{
    int fd = va_arg(ap, int);

    return VFSClientClose(fd);
}



static long sf_readv(va_list ap)
{

    int fd = va_arg(ap, int);
    const struct iovec *iov = va_arg(ap, struct iovec *);
    int iovcnt = va_arg(ap, int);
    long acc = 0;
    for(int i=0;i<iovcnt;i++)
    {
        struct iovec * v = iov + i;
        if(v->iov_len)
        {
            long r = VFSClientRead(fd, v->iov_base, v->iov_len);
            if(r<0)
            {
                Printf("sf_readv VFSClientRead error %li\n", r);                
                return r;
            }
            acc += r;
        }
    }

    return acc;
}
static long sf_read(va_list ap)
{
    int fd = va_arg(ap, int);
    char *buf = va_arg(ap, char *);
    int bufSize = va_arg(ap, int);

    return VFSClientRead(fd, buf, bufSize);
}

static long sf_lseek(va_list ap)
{
    int fd = va_arg(ap, int);
    off_t offset = va_arg(ap, off_t);
    int whence = va_arg(ap, int);

    return VFSClientSeek(fd, offset, whence);
}

static long sf_pread64(va_list ap)
{
    int fd = va_arg(ap, int);
    void *buf = va_arg(ap, void*);
    size_t count = va_arg(ap, size_t);
    off_t offset = va_arg(ap, off_t);

    off_t old_offset = lseek (fd, 0, SEEK_CUR);
    if (old_offset == (off_t) -1)
        return -1;
    /* Set to wanted position.  */
    if (lseek (fd, offset, SEEK_SET) == (off_t) -1)
    {
        return -1;
    }

    ssize_t result = read (fd, buf, count);

    int save_errno = errno;

    if (lseek (fd, old_offset, SEEK_SET) == (off_t) -1)
    {
        if (result == -1)
        {
            errno = save_errno;
        }
        return -1;
    }
    errno = save_errno;
    return result;
}


static long sf_open(va_list ap)
{
    const char *pathname = va_arg(ap, const char *);
    int flags = va_arg(ap, int);
    mode_t mode = va_arg(ap, mode_t);
    return VFSClientOpen(pathname, flags);
//    return 0;
}


static long sf_getdents64(va_list ap)
{
    unsigned int fd = va_arg(ap, int);
    struct dirent *dirp = va_arg(ap, struct dirent *);
    unsigned int count = va_arg(ap, unsigned int);

    ssize_t ret = VFSClientRead(fd, dirp, count);
    return ret;
    //On success, the number of bytes read is returned.
    //On end of directory, 0 is returned. On error, -1 is returned, 
    //and errno is set appropriately. 
}

static long sf_socket(va_list ap)
{
    int domain = va_arg(ap, int);
    int type = va_arg(ap, int);
    int protocol = va_arg(ap, int);

    return NetClientSocket(domain, type, protocol);
}

static long sf_bind(va_list ap)
{
    int sockfd = va_arg(ap, int);
    const struct sockaddr *addr = va_arg(ap, struct sockaddr *);
    socklen_t addrlen = va_arg(ap, socklen_t);
    return NetClientBind(sockfd, addr, addrlen);
}

static long sf_sendto(va_list ap)
{
    int sockfd = va_arg(ap, int);
    const void *buf = va_arg(ap, void*);
    size_t len = va_arg(ap, size_t);
    int flags = va_arg(ap, int);
    const struct sockaddr *dest_addr = va_arg(ap, struct sockaddr *);
    socklen_t addrlen = va_arg(ap, socklen_t);
    
    return NetClientSendTo(sockfd, buf, len, flags, dest_addr, addrlen);
}

static long sf_recvfrom(va_list ap)
{
    int sockfd = va_arg(ap, int);
    void *buf = va_arg(ap, void*);
    size_t len = va_arg(ap, size_t);
    int flags = va_arg(ap, int);
    struct sockaddr *src_addr = va_arg(ap, struct sockaddr *);
    socklen_t *addrlen = va_arg(ap, socklen_t *);
    return NetClientRecvFrom(sockfd, buf, len, flags, src_addr, addrlen);
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
    SFPrintf("Not implemented: fcntl for fd=%i cmd=%X\n", fd, cmd);
    return -1;
}

static long sf_wait4(va_list ap)
{
    pid_t pid = va_arg(ap, pid_t);
    int* status = va_arg(ap, int*);
    int options = va_arg(ap, int);
    struct rusage *rusage  = va_arg(ap, struct rusage *);
    return ProcClientWaitPid(pid, status, options);
    
}


static long sf_ioctl(va_list ap)
{
    int fd = va_arg(ap, int);
    unsigned long request = va_arg(ap, unsigned long);
    switch (request)
    {
    case TIOCGWINSZ:
    {
        struct winsize* arg = va_arg(ap, struct winsize*);
        return -1;
    }
    
    default:
        SFPrintf("Not implemented: ioctl for fd=%i request=%X\n", fd, request);
        return -1;
    }
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
        ret += VFSClientWrite(fildes, iov[i].iov_base, iov[i].iov_len);
    }
    return ret;
}

static long sf_write(va_list ap)
{
    int fd = va_arg(ap, int);
    const void *buf = va_arg(ap, void *);
    size_t bufSize = va_arg(ap, size_t);

    return VFSClientWrite(fd, buf, bufSize);
}


static long sf_getcwd(va_list ap)
{
    char* buf = va_arg(ap, char*);
    size_t size = va_arg(ap, size_t);

    return VFSClientGetCWD(buf, size);
}

static long sf_chdir(va_list ap)
{
    const char* path = va_arg(ap, char*);
    return VFSClientChDir(path);
}

static long sf_stat(va_list ap)
{
    const char *pathname = va_arg(ap, char*);
    struct stat *statbuf = va_arg(ap, struct stat *);
    return VFSClientStat(pathname, statbuf);
}

long sofa_vsyscall(long sysnum, ...)
{
    va_list al;
    va_start(al, sysnum);
    long ret = -1;
    switch (sysnum)
    {
    case __NR_stat:
        ret = sf_stat(al);
        break;
    case __NR_getpid:
        ret = SFGetPid();
        break;
    case __NR_getppid:
        ret = SFGetPPid();
        break;
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
    case __NR_readv:
        ret = sf_readv(al);
        break;
    case __NR_pread64:
        ret = sf_pread64(al);
        break;
    case __NR_lseek:
        ret = sf_lseek(al);
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
    case __NR_wait4:
        ret = sf_wait4(al);
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
    case __NR_socket:
        ret = sf_socket(al);
        break;
    case __NR_bind:
        ret = sf_bind(al);
        break;
    case __NR_sendto:
        ret = sf_sendto(al);
        break;
    case __NR_recvfrom:
        ret = sf_recvfrom(al);
        break;
    case __NR_getcwd:
        ret = sf_getcwd(al);
        break;
    case __NR_chdir:
        ret = sf_chdir(al);
        break;
    case __NR_access:
        SFPrintf("Dummy access syscall returning ok\n");
        return 0;
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