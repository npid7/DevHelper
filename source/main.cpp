#include <rd7.hpp>
#include <renderd7/log.hpp>

#include "Ovls.hpp"
#include "nightlyreader.hpp"
#include "scenes.hpp"

int main() {
  rd7_do_splash = true;
  // Never enable on Release Builds...
  rd7_enable_memtrack = false;

  RenderD7::Init::Main("Dev-Helper");
  rd7_security->SafeInit(amInit, amExit);
  RenderD7::Msg::Display("DevHelper", "Initializing...", Top);

  RenderD7::AddOvl(std::make_unique<Warnings>(
      "DevHelper",
      "Whwn loading data at some points\nthe app could randomly crash..."));

  RenderD7::Scene::Load(std::make_unique<DBSel>());
  while (RenderD7::MainLoop()) {
    if (d7_hDown & KEY_START) RenderD7::ExitApp();

    RenderD7::FrameEnd();
  }
}
