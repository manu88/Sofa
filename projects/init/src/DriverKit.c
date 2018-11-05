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

#include "DriverKit.h"

int DriverKitInit(InitContext* context)
{	
//	seL4_CPtr cap = simple_get_IOPort_cap(&context->simple, 1,1);
// cspace_irq_control_get_cap( simple_get_cnode(&context->simple)) , seL4_CapIRQControl, 1);

	return 1;
}
