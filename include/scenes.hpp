#include "nightlyreader.hpp"
#include "rd7.hpp"

enum MState { DB, APPV };
class DBSel : public RenderD7::Scene {
 public:
  DBSel();
  void Draw(void) const override;
  void Logic(u32 hDown, u32 hHeld, u32 hUp, touchPosition touch) override;

 private:
  int dirsel = 0;
  int SPos = 0;
  DBLoader dbld;
  MState state = DB;
  std::vector<RenderD7::TObject> buttons = {
      {20, 35, 120, 35, "Cia"},
      {20, 85, 120, 35, "3dsx"},
      {20, 135, 120, 35, "firm"},
      {20, 185, 120, 35, "cfwfirm"},
  };
};
