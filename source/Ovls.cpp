#include "Ovls.hpp"

Warnings::Warnings(std::string head, std::string sub) {
  this->_head = head;
  this->_sub = sub;
}

void Warnings::Draw(void) const {
  RenderD7::OnScreen(Top);
  RenderD7::Draw::Rect(0, msgposy, 400, 70, RenderD7::Color::Hex("#111111"));
  RenderD7::Draw::Rect(0, msgposy, 400, 25, RenderD7::Color::Hex("#222222"));
  RenderD7::Draw::Text(2, msgposy + 3, 0.7f, RenderD7::Color::Hex("#ffffff"),
                       _head);
  RenderD7::Draw::Text(2, msgposy + 30, 0.6f, RenderD7::Color::Hex("#ffffff"),
                       _sub);
}

void Warnings::Logic() {
  this->delay++ /*=(int)RenderD7::GetDeltaTime()*/;
  if (msgposy > 170 && delay < 2 * 60)
    msgposy-- /*=(int)RenderD7::GetDeltaTime()*/;

  if (delay >= 5 * 60) {
    msgposy++ /*=(int)RenderD7::GetDeltaTime*/;
    if (msgposy > 400) this->Kill();
  }
}

Errors::Errors(Result code) { this->rescode = code; }

void Errors::Draw(void) const {
  RenderD7::ResultDecoder resdec;
  resdec.Load(rescode);
  RenderD7::OnScreen(Top);
  RenderD7::Draw::Rect(0, msgposy, 400, 90, RenderD7::Color::Hex("#111111"));
  RenderD7::Draw::Rect(0, msgposy, 400, 25, RenderD7::Color::Hex("#222222"));
  RenderD7::Draw::Text(2, msgposy + 3, 0.7f, RenderD7::Color::Hex("#ffffff"),
                       "Error-Details: (" + resdec.GetCode() + ") ");
  RenderD7::Draw::Text(2, msgposy + 30, 0.6f, RenderD7::Color::Hex("#ffffff"),
                       "Module: " + resdec.GetModule() +
                           " - Level: " + resdec.GetLevel() +
                           "\nSummary: " + resdec.GetSummary() +
                           "\nDescription: " + resdec.GetDescription());
}

void Errors::Logic() {
  this->delay++ /*=(int)RenderD7::GetDeltaTime()*/;
  if (msgposy > 150 && delay < 2 * 60)
    msgposy-- /*=(int)RenderD7::GetDeltaTime()*/;

  if (delay >= 5 * 60) {
    msgposy++ /*=(int)RenderD7::GetDeltaTime*/;
    if (msgposy > 400) this->Kill();
  }
}