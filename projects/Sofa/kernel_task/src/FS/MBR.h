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


typedef struct 
{
    uint8_t active; // 0=no, 0x80 = bootable
    uint8_t startingHead;
    uint8_t startSector:6;
    uint8_t startCylinder_0:2;
    uint8_t startCylinder_1;

    uint8_t systemID;
    uint8_t endingHead;

    uint8_t endingSector:6;
    uint8_t endingCylinder_0:2;
    uint8_t endingCylinder_1;

    uint32_t lbaStart;
    uint32_t numSectors;

} __attribute__((packed)) PartitionTableEntry;


typedef struct 
{
    uint8_t code[440];
    uint32_t diskID;
    uint8_t reserved[2];
    PartitionTableEntry part1;
    PartitionTableEntry part2;
    PartitionTableEntry part3;
    PartitionTableEntry part4;
    uint16_t validBoot;
} __attribute__((packed)) MBR;