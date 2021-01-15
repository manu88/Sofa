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
#pragma once
#include <utils/uthash.h>
#include "IODevice.h"
#include "ext2.h"

int Ext2ReadBlock(uint8_t *buf, uint32_t blockID, IODevice *dev);
uint8_t* Ext2ReadBlockCached(uint32_t blockID, IODevice* dev);
uint8_t Ext2ReadInode(inode_t *inode_buf, uint32_t inode, IODevice *dev);