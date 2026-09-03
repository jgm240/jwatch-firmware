#include "components/settings/Settings.h"
#include <cstdlib>
#include <cstring>

using namespace Pinetime::Controllers;

Settings::Settings(Pinetime::Controllers::FS& fs) : fs {fs} {
}

void Settings::Init() {

  // Load default settings from Flash
  LoadSettingsFromFile();
}

void Settings::SaveSettings() {

  // verify if is necessary to save
  if (settingsChanged) {
    SaveSettingsToFile();
  }
  settingsChanged = false;
}

void Settings::LoadSettingsFromFile() {
  SettingsData bufferSettings;
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/settings.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&bufferSettings), sizeof(settings));
  fs.FileClose(&settingsFile);
  if (bufferSettings.version == settingsVersion) {
    settings = bufferSettings;
  }
}

void Settings::SaveSettingsToFile() {
  lfs_file_t settingsFile;

  // Write to a temp file, then atomically rename it over the real one, rather than
  // overwriting /settings.dat directly. A direct overwrite leaves only one copy of the
  // data on flash for the duration of the write - if power is actually cut mid-write
  // (a real battery disconnect/brownout, not a software reset: the external SPI-NOR
  // chip completes an already-issued page program on its own regardless of what the
  // MCU does afterward, but loses that guarantee if its own supply drops), that one
  // copy can end up torn/corrupted. LoadSettingsFromFile() then safely discards it as a
  // version mismatch, which reads back as a full reset to defaults. lfs_rename is
  // atomic, so the old file stays valid for the entire risky write window.
  if (fs.FileOpen(&settingsFile, "/settings.dat.tmp", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    return;
  }
  fs.FileWrite(&settingsFile, reinterpret_cast<uint8_t*>(&settings), sizeof(settings));
  fs.FileClose(&settingsFile);
  fs.Rename("/settings.dat.tmp", "/settings.dat");
}
