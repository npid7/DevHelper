#pragma once
#include <rd7.hpp>
#include <renderd7/log.hpp>

class Warnings : public RenderD7::Ovl {
 public:
  Warnings(std::string head, std::string sub);
  void Draw(void) const override;
  void Logic() override;

 private:
  std::string _head, _sub;
  int msgposy = 240;
  int delay = 0;
};

class Errors : public RenderD7::Ovl {
 public:
  Errors(Result code);
  void Draw(void) const override;
  void Logic() override;

 private:
  Result rescode;
  int msgposy = 240;
  int delay = 0;
};
