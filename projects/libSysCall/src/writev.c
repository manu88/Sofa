/*
 * This file is part of the Sofa project
 * Copyright (c) 2018 Manuel Deneu.
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

#include <limits.h>
#include <sys/uio.h>
#include <unistd.h>
#include "SysCallsList.h"



long sofa_writev(va_list args)
{
        int fildes        = va_arg(args, int);
        struct iovec *iov = va_arg(args, struct iovec *);
        int iovcnt        = va_arg(args, int);

        long long sum = 0;
        ssize_t ret = 0;
        return 0;
        
/*
        // The iovcnt argument is valid if greater than 0 and less than or equal to IOV_MAX. 
        if (iovcnt <= 0 || iovcnt > IOV_MAX) {
                return -EINVAL;
        }
*/
        // The sum of iov_len is valid if less than or equal to SSIZE_MAX i.e. cannot overflow a ssize_t. 
        for (int i = 0; i < iovcnt; i++) 
        {
                sum += (long long)iov[i].iov_len;
                if (sum > SSIZE_MAX) {
                        return -EINVAL;
                }
        }

        // If all the iov_len members in the array are 0, return 0. 
        if (!sum) 
        {
                return 0;
        }

        for (int i = 0; i < iovcnt; i++) 
        {
                ret += write( fildes,iov[i].iov_base, iov[i].iov_len);
        }

        return ret;

}
