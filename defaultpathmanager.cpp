
#include "defaultpathmanager.h"

DefaultPathManager::DefaultPathManager() {}

void DefaultPathManager::add(QString ID, QString path) { defaultPaths[ID] = path; }
