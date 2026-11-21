#include "displayapp/screens/JwafApp.h"
#include <lvgl/lvgl.h>
#include <littlefs/lfs.h>
#include <cstdio>
#include <cstring>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;
using Pinetime::Controllers::JwafFormat::kHeaderSize;
using Pinetime::Controllers::JwafFormat::kFormatVersion;
using Pinetime::Controllers::JwafFormat::LineMaxLength;
using Pinetime::Controllers::JwafFormat::MaxLines;
using Pinetime::Controllers::JwafFormat::TitleMaxLength;

namespace {
  constexpr const char* appsDir = "/apps";
  // Biggest a file is allowed to be once header limits are maxed out: title + every line at
  // its max length, each with its own 1-byte length prefix.
  constexpr size_t kMaxFileSize = kHeaderSize + TitleMaxLength + MaxLines * (1 + LineMaxLength);

  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<JwafApp*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  void StyleListButton(lv_obj_t* btn) {
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_text_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }

  // Bounds-checked read of a length-prefixed [len byte][len bytes of text] field. Returns the
  // read length, or -1 if the field's own length byte would run past the buffer - a corrupt or
  // truncated file can claim any length byte, so this has to be checked before copying.
  int ReadLengthPrefixed(const uint8_t* data, size_t size, size_t offset, uint8_t maxLen, char* out, size_t outCap) {
    if (offset >= size) {
      return -1;
    }
    uint8_t len = data[offset];
    if (len > maxLen || offset + 1 + len > size) {
      return -1;
    }
    size_t copyLen = len < outCap - 1 ? len : outCap - 1;
    std::memcpy(out, data + offset + 1, copyLen);
    out[copyLen] = '\0';
    return static_cast<int>(1 + len);
  }
}

JwafApp::JwafApp(Pinetime::Controllers::FS& filesystem) : filesystem {filesystem} {
  ShowList();
}

JwafApp::~JwafApp() {
  lv_obj_clean(lv_scr_act());
}

void JwafApp::LoadFileList() {
  fileCount = 0;
  lfs_dir_t dir;
  if (filesystem.DirOpen(appsDir, &dir) < 0) {
    return;
  }
  lfs_info info;
  while (fileCount < MaxFiles && filesystem.DirRead(&dir, &info) > 0) {
    if (info.type != LFS_TYPE_REG) {
      continue;
    }
    size_t len = std::strlen(info.name);
    if (len < 6 || std::strcmp(info.name + len - 5, ".jwaf") != 0) {
      continue;
    }
    strncpy(files[fileCount].name, info.name, sizeof(files[fileCount].name) - 1);
    files[fileCount].name[sizeof(files[fileCount].name) - 1] = '\0';
    fileCount++;
  }
  filesystem.DirClose(&dir);
}

void JwafApp::ShowList() {
  mode = Mode::List;
  lv_obj_clean(lv_scr_act());
  titleLabel = nullptr;
  errorLabel = nullptr;
  lineLabels.fill(nullptr);

  lv_obj_t* header = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(header, "Apps");
  lv_obj_align(header, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 5);

  LoadFileList();

  list = lv_list_create(lv_scr_act(), nullptr);
  lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - 30);
  lv_obj_align(list, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  if (fileCount == 0) {
    StyleListButton(lv_list_add_btn(list, nullptr, "(none installed)"));
    return;
  }
  for (int i = 0; i < fileCount; i++) {
    lv_obj_t* btn = lv_list_add_btn(list, nullptr, files[i].name);
    StyleListButton(btn);
    btn->user_data = this;
    lv_obj_set_event_cb(btn, eventHandler);
    buttonObjects[i] = btn;
  }
}

void JwafApp::ShowError(const char* message) {
  mode = Mode::Error;
  lv_obj_clean(lv_scr_act());
  list = nullptr;

  lv_obj_t* back = lv_list_add_btn(lv_scr_act(), nullptr, "< Apps");
  lv_obj_set_size(back, LV_HOR_RES, 35);
  lv_obj_align(back, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  StyleListButton(back);
  back->user_data = this;
  lv_obj_set_event_cb(back, eventHandler);
  buttonObjects[0] = back;

  errorLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(errorLabel, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(errorLabel, LV_HOR_RES - 20);
  lv_obj_set_style_local_text_color(errorLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_text(errorLabel, message);
  lv_obj_align(errorLabel, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, 50);
}

void JwafApp::OpenFile(const char* name) {
  char path[64];
  snprintf(path, sizeof(path), "%s/%s", appsDir, name);

  lfs_file_t file;
  if (filesystem.FileOpen(&file, path, LFS_O_RDONLY) < 0) {
    ShowError("Could not open that file.");
    return;
  }

  uint8_t buffer[kMaxFileSize];
  int readLen = filesystem.FileRead(&file, buffer, sizeof(buffer));
  filesystem.FileClose(&file);

  if (readLen < static_cast<int>(kHeaderSize)) {
    ShowError("File is too short to be a JWAF app.");
    return;
  }
  size_t size = static_cast<size_t>(readLen);

  if (!Pinetime::Controllers::JwafFormat::HasValidMagic(buffer, size)) {
    ShowError("Not a valid JWAF file (bad header).");
    return;
  }
  using Pinetime::Controllers::JwafFormat::kMagicLength;
  if (buffer[kMagicLength] != kFormatVersion) {
    ShowError("This JWAF file needs a newer firmware.");
    return;
  }

  uint8_t titleLen = buffer[kMagicLength + 1];
  uint8_t lineCount = buffer[kMagicLength + 2];
  if (titleLen > TitleMaxLength || lineCount > MaxLines) {
    ShowError("JWAF file is corrupt (header out of range).");
    return;
  }

  char titleText[TitleMaxLength + 1];
  size_t offset = kHeaderSize;
  if (offset + titleLen > size) {
    ShowError("JWAF file is corrupt (truncated title).");
    return;
  }
  std::memcpy(titleText, buffer + offset, titleLen);
  titleText[titleLen] = '\0';
  offset += titleLen;

  char lineText[MaxLines][LineMaxLength + 1];
  for (uint8_t i = 0; i < lineCount; i++) {
    int consumed = ReadLengthPrefixed(buffer, size, offset, LineMaxLength, lineText[i], sizeof(lineText[i]));
    if (consumed < 0) {
      ShowError("JWAF file is corrupt (truncated line).");
      return;
    }
    offset += static_cast<size_t>(consumed);
  }

  mode = Mode::Viewing;
  lv_obj_clean(lv_scr_act());
  list = nullptr;
  errorLabel = nullptr;

  lv_obj_t* back = lv_list_add_btn(lv_scr_act(), nullptr, "< Apps");
  lv_obj_set_size(back, LV_HOR_RES, 30);
  lv_obj_align(back, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  StyleListButton(back);
  back->user_data = this;
  lv_obj_set_event_cb(back, eventHandler);
  buttonObjects[0] = back;

  titleLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(titleLabel, titleText);
  lv_obj_align(titleLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 35);

  lv_coord_t rowY = 65;
  for (uint8_t i = 0; i < lineCount; i++) {
    lv_obj_t* lbl = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_long_mode(lbl, LV_LABEL_LONG_DOT);
    lv_obj_set_width(lbl, LV_HOR_RES - 20);
    lv_label_set_text(lbl, lineText[i]);
    lv_obj_align(lbl, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 10, rowY);
    lineLabels[i] = lbl;
    rowY += 25;
  }
}

void JwafApp::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (mode == Mode::Viewing || mode == Mode::Error) {
    if (obj == buttonObjects[0]) {
      ShowList();
    }
    return;
  }
  for (int i = 0; i < fileCount; i++) {
    if (buttonObjects[i] == obj) {
      OpenFile(files[i].name);
      return;
    }
  }
}
