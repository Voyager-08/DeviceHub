#pragma once

#include <QString>

namespace devicehub {

// 插件与主程序之间约定的 API 版本号
inline constexpr int kApiVersion = 1;

// 核心库版本字符串
QString coreVersion();

} // namespace devicehub
