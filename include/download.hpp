#include <3ds.h>

#include <cstring>
#include <iostream>
#include <string>

#define APP_TITLE "DevHelper"
#define VERSION_STRING "1.0.0"

enum DownloadError {
  DL_ERROR_NONE = 0,
  DL_ERROR_WRITEFILE,
  DL_ERROR_ALLOC,
  DL_ERROR_STATUSCODE,
  DL_ERROR_GIT,
  DL_CANCEL,  // No clue if that's needed tho.
};

Result downloadToFile(const std::string &url, const std::string &path);
// Result downloadFromRelease(std::string url, std::string asset, std::string
// path, bool includePrereleases);
void displayProgressBar();
bool checkWifiStatus(void);