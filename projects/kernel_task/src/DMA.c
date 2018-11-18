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


#include <sel4utils/page_dma.h>
#include "DMA.h"
#include <assert.h>

int DMAInit( KernelTaskContext* context)
{
	int ret = sel4utils_new_page_dma_alloc( &context->vka, &context->vspace, &context->opsIO.dma_manager) == 0;

	assert(context->opsIO.dma_manager.dma_alloc_fn != NULL);

	return ret;
}
