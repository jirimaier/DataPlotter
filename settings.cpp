#include "settings.h"

Settings::Settings() {
  for (int i = 0; i < CHANNEL_COUNT; i++)
    channelSettings.append(new channelSettings_t);
  for (int i = 0; i < 8; i++)
    channelSettings.at(i)->color = defaultColors[i];
}
