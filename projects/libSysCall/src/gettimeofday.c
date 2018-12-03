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

#include "SysCallsList.h"

// int gettimeofday(struct timeval *tv, struct timezone *tz);
long sofa_gettimeofday(va_list args)
{
        struct timeval *tv  = va_arg (args, struct timeval* );
        struct timezone *tz = va_arg (args, struct timezone* );

	assert(0 && "gettimeofday not implemented");
        return 0;
}
