
//
//  hal_directory_win.cpp
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "hal_directory_win.h"
#include <stdio.h>
#include <fcntl.h>

#ifdef PLATFORM_WIN

#include <shlobj.h>
#define chmod _chmod
#define mkdir _mkdir
#define rmdir _rmdir
#define ssize_t SSIZE_T

//
// Windows support for reading directories
//

DIR* opendir(const char* name)
{
    DIR* dir = 0;
    if (name && name[0]) {
        size_t base_length = strlen(name);
        // search pattern must end with suitable wildcard
        const char* all = strchr("/\\", name[base_length - 1]) ? "*" : "\\*";
        size_t all_len = base_length + strlen(all);
        if ((dir = (DIR*) malloc(sizeof *dir)) != 0 &&
           (dir->name = (char*) malloc(all_len + 1)) != 0)
        {
            strcat(strcpy(dir->name, name), all);
            if ((dir->handle = (handle_type) _findfirst(dir->name, &dir->info)) != -1) {
                dir->result.d_name = 0;
                dir->result.d_type = DT_DIR;
            } else {
                free(dir->name);
                free(dir);
                dir = 0;
            }
        } else {
            free(dir);
            dir = 0;
            errno = ENOMEM;
        }
    } else {
        errno = EINVAL;
    }
    return dir;
}

int closedir(DIR* dir)
{
    int result = -1;
    if (dir) {
        if (dir->handle != -1)
            result = _findclose(dir->handle);
        free(dir->name);
        free(dir);
    }
    if (result == -1)
        errno = EBADF;
    return result;
}

struct dirent* readdir(DIR* dir)
{
    struct dirent* result = 0;
    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {
            result = &dir->result;
            result->d_name = dir->info.name;
            result->d_type = DT_DIR;
        }
    } else {
        errno = EBADF;
    }
    return result;
}

void rewinddir(DIR* dir)
{
    if (dir && dir->handle != -1) {
        _findclose(dir->handle);
        dir->handle = (handle_type) _findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
        dir->result.d_type = 0;
    } else {
        errno = EBADF;
    }
}

#endif
