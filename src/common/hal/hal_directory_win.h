
//
//  hal_directory_win.h
//
//  Created by Christian Lehner on 11/18/15.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#ifndef SOFTHUB_HAL_DIRECTORY_WIN_H
#define SOFTHUB_HAL_DIRECTORY_WIN_H

#include <base/base.h>
#include <sys/stat.h>
#ifdef PLATFORM_WIN
#include <windows.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define DT_DIR		 4
#define DT_REG		 8
#define DT_LNK		10

#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m) (false)

struct dirent {
    char* d_name;
    int d_type;
};

typedef ptrdiff_t handle_type;

struct DIR {
    handle_type handle;
    struct _finddata_t info;
    struct dirent result;
    char* name;
};

extern DIR* opendir(const char* name);
extern int closedir(DIR* dir);
extern struct dirent* readdir(DIR* dir);

#endif
#endif
