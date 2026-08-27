#include "displayapp/widgets/complications/NotificationIconComplication.h"
#include "displayapp/screens/NotificationIcon.h"

using namespace Pinetime::Applications::Widgets;

NotificationIconComplication::NotificationIconComplication(const Controllers::NotificationManager& notificationManager)
  : notificationManager {notificationManager} {
}

void NotificationIconComplication::Create(lv_obj_t* parent, lv_align_t align, lv_coord_t xOfs, lv_coord_t yOfs) {
  label = lv_label_create(parent, nullptr);
  lv_label_set_text_static(label, Screens::NotificationIcon::GetIcon(false));
  lv_obj_align(label, parent, align, xOfs, yOfs);
}

void NotificationIconComplication::Update() {
  notificationState = notificationManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(label, Screens::NotificationIcon::GetIcon(notificationState.Get()));
    lv_obj_realign(label);
  }
}
