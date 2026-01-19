// FROM UNIVERSAL-UPDATER 2.5.1
#pragma once
#include <3ds.h>

#include "Files.hpp"

Result CIA_LaunchTitle(u64 titleId, FS_MediaType mediaType);
Result deletePrevious(u64 titleid, FS_MediaType media);
Result installCia(const char *ciaPath, bool updateSelf);