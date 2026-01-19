#pragma once
#include <renderd7/ini.hpp>
#include <string>
#include <vector>

#include "Ovls.hpp"

#define D_P()                                                       \
  std::cout << "\u001b[33;1mBreakpoint reached: \n"                 \
            << (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 \
                                       : __FILE__)                  \
            << ":" << __LINE__ << "\u001b[0m\n";                    \
  for (int i = 0; i < 3 * 60; i++)
struct DB_Entry {
  std::string name;
  std::string dl_link;
};
struct DB {
  std::string reponame;
  std::string repo_host;
  std::vector<DB_Entry> e_list;
};

struct APPH {
  std::string Name;
  std::string author;
  std::string commit_tag;
  std::string desc;
  std::string ver;
  std::string dl_3dsx;
  std::string dl_cia;
  bool dll_3dsx = true;
  bool dll_cia = true;
};

class DBLoader {
 public:
  DBLoader() {}
  ~DBLoader() {}
  void LoadDB(std::string link);
  void DownloadEntry(int index);
  void Download3dsx(int index);
  void InstallCia(int index);
  void LoadEntry(int index);
  void DownloadApp(int index, bool _3dsx = true);
  std::string GetRepoName() { return this->db.reponame; }
  std::string GetRepoHost() { return this->db.repo_host; }
  DB db;
  std::vector<APPH> versions;

 private:
  std::vector<std::string> secs;
  std::vector<std::string> appsecs;
};
