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
#include <sys/types.h>

typedef enum
{
    ProcRequest_Register,
    ProcRequest_Enum,
    ProcRequest_Kill,
    ProcRequest_Spawn,
    ProcRequest_Wait,
}ProcRequest;

typedef struct
{
    pid_t pid;
    uint64_t startTime;
    uint8_t state;
    uint16_t nameLen;
    uint64_t numPages;
    char name[]; // to keep as last field as the name len is variable!
}ProcessDesc;


extern const char procServiceName[];

int ProcClientInit(void);

// returning other than 0 will stop the enum.
typedef int (*OnProcessDescription)(const ProcessDesc* p, void* ptr);

int ProcClientEnum(OnProcessDescription callb, void* ptr);
int ProcClientKill(pid_t pid, int sig);
int ProcClientSpawn(const char* buf);

pid_t ProcClientWaitPid(pid_t pid, int *wstatus, int options);
pid_t ProcClientWait(int *wstatus);
