#include "download.hpp"

#include <3ds.h>
#include <curl/curl.h>
#include <dirent.h>
#include <malloc.h>
#include <unistd.h>

#include <rd7.hpp>
#include <regex>
#include <string>
#include <vector>

#define USER_AGENT APP_TITLE "-" VERSION_STRING

#define TIME_IN_US 1
#define TIMETYPE curl_off_t
#define TIMEOPT CURLINFO_TOTAL_TIME_T
#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL 3000000

CURL *CURL_HND;

curl_off_t downloadTotal =
    1;  // Dont initialize with 0 to avoid division by zero later.
curl_off_t downloadNow = 0;
curl_off_t downloadSpeed = 0;

float DLTotal = 1;  // Dont initialize with 0 to avoid division by zero later.
float DLNow = 0;
float DLSpeed = 0;

float bldl = false;

static FILE *downfile = nullptr;
static size_t file_buffer_pos = 0;
static size_t file_toCommit_size = 0;
static char *g_buffers[2] = {nullptr};
static u8 g_index = 0;
static Thread fsCommitThread;
static LightEvent readyToCommit;
static LightEvent waitCommit;
static bool killThread = false;
static bool writeError = false;
#define FILE_ALLOC_SIZE 0x60000

int progressBarType = 0;
char progressBarMsg[128] = "";
bool showProgressBar = false;

/* That are our install Progressbar variables. */
extern u64 installSize, installOffset;

static int curlProgress(CURL *hnd, curl_off_t dltotal, curl_off_t dlnow,
                        curl_off_t ultotal, curl_off_t ulnow) {
  downloadTotal = dltotal;
  downloadNow = dlnow;
  DLTotal = downloadTotal;
  DLNow = downloadNow;

  return 0;
}

bool filecommit() {
  if (!downfile) return false;
  fseek(downfile, 0, SEEK_END);
  u32 byteswritten =
      fwrite(g_buffers[!g_index], 1, file_toCommit_size, downfile);
  if (byteswritten != file_toCommit_size) return false;
  file_toCommit_size = 0;
  return true;
}

static void commitToFileThreadFunc(void *args) {
  LightEvent_Signal(&waitCommit);

  while (true) {
    LightEvent_Wait(&readyToCommit);
    LightEvent_Clear(&readyToCommit);
    if (killThread) threadExit(0);
    writeError = !filecommit();
    LightEvent_Signal(&waitCommit);
  }
}

static size_t file_handle_data(char *ptr, size_t size, size_t nmemb,
                               void *userdata) {
  (void)userdata;
  const size_t bsz = size * nmemb;
  size_t tofill = 0;
  if (writeError) return 0;

  if (!g_buffers[g_index]) {
    LightEvent_Init(&waitCommit, RESET_STICKY);
    LightEvent_Init(&readyToCommit, RESET_STICKY);

    s32 prio = 0;
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    fsCommitThread =
        threadCreate(commitToFileThreadFunc, NULL, 0x1000, prio - 1, -2, true);

    g_buffers[0] = (char *)memalign(0x1000, FILE_ALLOC_SIZE);
    g_buffers[1] = (char *)memalign(0x1000, FILE_ALLOC_SIZE);

    if (!fsCommitThread || !g_buffers[0] || !g_buffers[1]) return 0;
  }

  if (file_buffer_pos + bsz >= FILE_ALLOC_SIZE) {
    tofill = FILE_ALLOC_SIZE - file_buffer_pos;
    memcpy(g_buffers[g_index] + file_buffer_pos, ptr, tofill);

    LightEvent_Wait(&waitCommit);
    LightEvent_Clear(&waitCommit);
    file_toCommit_size = file_buffer_pos + tofill;
    file_buffer_pos = 0;
    svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (u32)g_buffers[g_index],
                             file_toCommit_size);
    g_index = !g_index;
    LightEvent_Signal(&readyToCommit);
  }

  memcpy(g_buffers[g_index] + file_buffer_pos, ptr + tofill, bsz - tofill);
  file_buffer_pos += bsz - tofill;
  return bsz;
}

Result downloadToFile(const std::string &url, const std::string &path) {
  downloadTotal = 1;
  downloadNow = 0;
  downloadSpeed = 0;

  CURLcode curlResult;

  Result retcode = 0;
  int res;

  printf("Downloading from:\n%s\nto:\n%s\n", url.c_str(), path.c_str());

  void *socubuf = memalign(0x1000, 0x100000);
  if (!socubuf) {
    retcode = -1;
    goto exit;
  }

  res = socInit((u32 *)socubuf, 0x100000);
  if (R_FAILED(res)) {
    retcode = res;
    goto exit;
  }

  /* make directories. */
  for (char *slashpos = strchr(path.c_str() + 1, '/'); slashpos != NULL;
       slashpos = strchr(slashpos + 1, '/')) {
    char bak = *(slashpos);
    *(slashpos) = '\0';

    mkdir(path.c_str(), 0777);

    *(slashpos) = bak;
  }

  downfile = fopen(path.c_str(), "wb");
  if (!downfile) {
    retcode = -2;
    goto exit;
  }
  printf("done!");

  CURL_HND = curl_easy_init();
  curl_easy_setopt(CURL_HND, CURLOPT_BUFFERSIZE, FILE_ALLOC_SIZE);
  curl_easy_setopt(CURL_HND, CURLOPT_URL, url.c_str());
  curl_easy_setopt(CURL_HND, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(CURL_HND, CURLOPT_USERAGENT, USER_AGENT);
  curl_easy_setopt(CURL_HND, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(CURL_HND, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(CURL_HND, CURLOPT_ACCEPT_ENCODING, "gzip");
  curl_easy_setopt(CURL_HND, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(CURL_HND, CURLOPT_XFERINFOFUNCTION, curlProgress);
  curl_easy_setopt(CURL_HND, CURLOPT_HTTP_VERSION,
                   (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(CURL_HND, CURLOPT_WRITEFUNCTION, file_handle_data);
  curl_easy_setopt(CURL_HND, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(CURL_HND, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(CURL_HND, CURLOPT_STDERR, stdout);

  bldl = 1;

  curlResult = curl_easy_perform(CURL_HND);
  curl_easy_cleanup(CURL_HND);

  bldl = 0;

  if (curlResult != CURLE_OK) {
    retcode = -curlResult;

    goto exit;
  }

  LightEvent_Wait(&waitCommit);
  LightEvent_Clear(&waitCommit);

  file_toCommit_size = file_buffer_pos;
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (u32)g_buffers[g_index],
                           file_toCommit_size);
  g_index = !g_index;

  if (!filecommit()) {
    retcode = -3;

    goto exit;
  }

  fflush(downfile);

exit:
  if (fsCommitThread) {
    killThread = true;
    LightEvent_Signal(&readyToCommit);
    threadJoin(fsCommitThread, U64_MAX);
    killThread = false;
    fsCommitThread = nullptr;
  }

  socExit();

  if (socubuf) free(socubuf);

  if (downfile) {
    fclose(downfile);
    downfile = nullptr;
  }

  if (g_buffers[0]) {
    free(g_buffers[0]);
    g_buffers[0] = nullptr;
  }

  if (g_buffers[1]) {
    free(g_buffers[1]);
    g_buffers[1] = nullptr;
  }

  g_index = 0;
  file_buffer_pos = 0;
  file_toCommit_size = 0;
  writeError = false;

  return retcode;
}

/* adapted from GM9i's byte parsing. */
std::string formatBytes(int bytes) {
  char out[32];

  if (bytes == 1)
    snprintf(out, sizeof(out), "%d Byte", bytes);

  else if (bytes < 1024)
    snprintf(out, sizeof(out), "%d Bytes", bytes);

  else if (bytes < 1024 * 1024)
    snprintf(out, sizeof(out), "%.1f KB", (float)bytes / 1024);

  else if (bytes < 1024 * 1024 * 1024)
    snprintf(out, sizeof(out), "%.1f MB", (float)bytes / 1024 / 1024);

  else
    snprintf(out, sizeof(out), "%.1f GB", (float)bytes / 1024 / 1024 / 1024);

  return out;
}

/* adapted from GM9i's byte parsing. */
std::string formatSBytes(int bytes) {
  char out[32];

  if (bytes == 1)
    snprintf(out, sizeof(out), "%d Byte/s", bytes);

  else if (bytes < 1024)
    snprintf(out, sizeof(out), "%d Bytes/s", bytes);

  else if (bytes < 1024 * 1024)
    snprintf(out, sizeof(out), "%.1f kb/s", (float)bytes / 1024);

  else if (bytes < 1024 * 1024 * 1024)
    snprintf(out, sizeof(out), "%.1f mb/s", (float)bytes / 1024 / 1024);

  else
    snprintf(out, sizeof(out), "%.1f gb/s", (float)bytes / 1024 / 1024 / 1024);

  return out;
}

// Stolen from tvx
std::string TimeFormat(int seconds) {
  std::stringstream ss;
  if (seconds > 3599) {
    ss << std::to_string(seconds / 60 / 60);
    ss << "h ";
  }
  if (seconds > 59) {
    ss << std::to_string(seconds / 60 % 60);
    ss << "m ";
  }

  ss << std::to_string(seconds % 60);
  ss << "s ";

  return ss.str();
}

std::string GetTimeNeed(float current, float total, float speed) {
  float timez = ((total - current) / speed);
  if (timez >= 0 && speed > 0) {
    return TimeFormat(timez);
  }
  return "Calculating...";
}

void displayProgressBar() {
  char str[256];
  while (showProgressBar) {
    if (bldl) {
      curl_easy_getinfo(CURL_HND, CURLINFO_SPEED_DOWNLOAD_T, &downloadSpeed);
    } else {
      downloadSpeed = 0;
    }
    DLSpeed = downloadSpeed;
    if (DLTotal < 1.0f) DLTotal = 1.0f;

    if (DLTotal < DLNow) DLTotal = DLNow;

    if (progressBarType == 0) {
      snprintf(str, sizeof(str), "%s / %s (%.2f%%)\nSpeed: %s\nETA: %s",
               formatBytes(DLNow).c_str(), formatBytes(DLTotal).c_str(),
               ((float)DLNow / (float)DLTotal) * 100.0f,
               formatSBytes(DLSpeed).c_str(),
               GetTimeNeed(DLNow, DLTotal, DLSpeed).c_str());

    } else {
      snprintf(str, sizeof(str), "%s / %s (%.2f%%)",
               formatBytes(installOffset).c_str(),
               formatBytes(installSize).c_str(),
               ((float)installOffset / (float)installSize) * 100.0f);
    };

    RenderD7::ClearTextBufs();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C2D_TargetClear(Top, RenderD7::Color::Hex("#000000"));
    C2D_TargetClear(Bottom, RenderD7::Color::Hex("#000000"));
    RenderD7::OnScreen(Top);

    RenderD7::OnScreen(Top);
    RenderD7::Draw::Rect(0, 0, 400, 240, RenderD7::Color::Hex("#111111"));
    RenderD7::Draw::Rect(0, 0, 400, 21, RenderD7::Color::Hex("#333333", 200));
    RenderD7::Draw::Text(5, 0, 0.7f, RenderD7::Color::Hex("#ffffff"),
                         APP_TITLE);
    RenderD7::Draw::Text(5, 160, 0.6f, RenderD7::Color::Hex("#ffffff"),
                         progressBarMsg);
    RenderD7::Draw::Rect(30, 200, 342, 16, RenderD7::Color::Hex("#333333"));

    /* Download. */
    if (progressBarType == 0) {
      RenderD7::Draw::Rect(31, 201,
                           (int)(((float)DLNow / (float)DLTotal) * 338.0f), 14,
                           RenderD7::Color::Hex("#00ff11"));

      /* Install. */
    } else {
      RenderD7::Draw::Rect(
          31, 201, (int)(((float)installOffset / (float)installSize) * 338.0f),
          14, RenderD7::Color::Hex("#00ff11"));
    }
    RenderD7::Draw::TextCentered(5, 50, 0.7f, RenderD7::Color::Hex("#ffffff"),
                                 str, 390);

    RenderD7::OnScreen(Bottom);
    RenderD7::Draw::Rect(0, 0, 320, 240, RenderD7::Color::Hex("#111111"));
    C3D_FrameEnd(0);
    gspWaitForVBlank();
  }
  C3D_FrameBegin(C3D_FRAME_NONBLOCK);
}

bool checkWifiStatus(void) {
  u32 wifiStatus;
  bool res = false;

  if (R_SUCCEEDED(ACU_GetWifiStatus(&wifiStatus)) && wifiStatus) res = true;

  return res;
}