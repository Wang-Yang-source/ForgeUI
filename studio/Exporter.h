#pragma once

#include <QString>

#include "SceneModel.h"

namespace forgeui_studio {

QString exportForgeUiSource(const SceneModel& scene);
QString exportLvglSource(const SceneModel& scene);
QString exportSlintSource(const SceneModel& scene);

} // namespace forgeui_studio
