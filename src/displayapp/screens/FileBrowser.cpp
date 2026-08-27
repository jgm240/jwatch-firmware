#include "displayapp/screens/FileBrowser.h"
#include <lvgl/lvgl.h>
#include <cstdio>
#include <cstring>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FileBrowser*>(obj->user_data);
    screen->OnEvent(obj, event);
  }

  // lv_list_add_btn's rows never went through Styles::Button() like every other button in
  // this app, so they were left on LVGL's stock lv_list theme instead of this app's own -
  // stock background and stock (inherited-white) text ended up the same color.
  void StyleListButton(lv_obj_t* btn) {
    lv_obj_set_style_local_bg_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);
    lv_obj_set_style_local_text_color(btn, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }
}

FileBrowser::FileBrowser(Pinetime::Controllers::FS& filesystem) : filesystem {filesystem} {
  pathLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(pathLabel, LV_LABEL_LONG_SROLL_CIRC);
  lv_obj_set_width(pathLabel, LV_HOR_RES);
  lv_obj_set_style_local_text_color(pathLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_align(pathLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  list = lv_list_create(lv_scr_act(), nullptr);
  lv_obj_set_size(list, LV_HOR_RES, LV_VER_RES - 30);
  lv_obj_align(list, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  LoadDirectory();
}

FileBrowser::~FileBrowser() {
  lv_obj_clean(lv_scr_act());
}

void FileBrowser::NavigateInto(const Entry& entry) {
  char newPath[256];
  if (strcmp(currentPath, "/") == 0) {
    snprintf(newPath, sizeof(newPath), "/%s", entry.name);
  } else {
    snprintf(newPath, sizeof(newPath), "%s/%s", currentPath, entry.name);
  }
  strncpy(currentPath, newPath, sizeof(currentPath) - 1);
  currentPath[sizeof(currentPath) - 1] = '\0';
  LoadDirectory();
}

void FileBrowser::NavigateUp() {
  if (strcmp(currentPath, "/") == 0) {
    return;
  }
  char* lastSlash = strrchr(currentPath, '/');
  if (lastSlash == currentPath) {
    currentPath[1] = '\0';
  } else if (lastSlash != nullptr) {
    *lastSlash = '\0';
  }
  LoadDirectory();
}

void FileBrowser::LoadDirectory() {
  entryCount = 0;

  lfs_dir_t dir;
  if (filesystem.DirOpen(currentPath, &dir) >= 0) {
    lfs_info info;
    while (entryCount < maxEntries && filesystem.DirRead(&dir, &info) > 0) {
      if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) {
        continue;
      }
      Entry& e = entries[entryCount];
      strncpy(e.name, info.name, sizeof(e.name) - 1);
      e.name[sizeof(e.name) - 1] = '\0';
      e.isDir = (info.type == LFS_TYPE_DIR);
      e.size = info.size;
      entryCount++;
    }
    filesystem.DirClose(&dir);
  }

  lv_list_clean(list);
  totalRows = 0;
  hasUpRow = strcmp(currentPath, "/") != 0;

  if (hasUpRow) {
    lv_obj_t* btn = lv_list_add_btn(list, nullptr, "../");
    StyleListButton(btn);
    btn->user_data = this;
    lv_obj_set_event_cb(btn, eventHandler);
    buttonObjects[totalRows++] = btn;
  }

  if (entryCount == 0) {
    StyleListButton(lv_list_add_btn(list, nullptr, "(empty)"));
  } else {
    for (int i = 0; i < entryCount; i++) {
      char label[80];
      if (entries[i].isDir) {
        snprintf(label, sizeof(label), "%s/", entries[i].name);
      } else {
        snprintf(label, sizeof(label), "%s (%lu B)", entries[i].name, static_cast<unsigned long>(entries[i].size));
      }
      lv_obj_t* btn = lv_list_add_btn(list, nullptr, label);
      StyleListButton(btn);
      btn->user_data = this;
      lv_obj_set_event_cb(btn, eventHandler);
      buttonObjects[totalRows++] = btn;
    }
  }

  lv_label_set_text(pathLabel, currentPath);
  lv_obj_realign(pathLabel);
}

void FileBrowser::OnEvent(lv_obj_t* obj, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  for (int i = 0; i < totalRows; i++) {
    if (buttonObjects[i] != obj) {
      continue;
    }
    if (hasUpRow && i == 0) {
      NavigateUp();
    } else {
      int entryIndex = i - (hasUpRow ? 1 : 0);
      if (entries[entryIndex].isDir) {
        NavigateInto(entries[entryIndex]);
      }
    }
    return;
  }
}
