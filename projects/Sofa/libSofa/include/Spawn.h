#pragma once
#include <sys/types.h> // pid_t

typedef struct
{
    /* data */
}posix_spawn_file_actions_t;

typedef struct
{
    /* data */
}posix_spawnattr_t;

int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);
