#pragma once
#include <3ds.h>

#include <cctype>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define WORKING_DIR "/"

Result makeDirs(const char *path);
Result openFile(Handle *fileHandle, const char *path, bool write);
Result deleteFile(const char *path);
Result removeDir(const char *path);
Result removeDirRecursive(const char *path);