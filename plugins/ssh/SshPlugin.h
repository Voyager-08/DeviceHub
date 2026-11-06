#pragma once

// SSH 插件骨架。
// 第 2 步定义 core 的 IPlugin 接口后,这里将改为实现 IPlugin 的插件类。
namespace devicehub::ssh {

const char *pluginId();

} // namespace devicehub::ssh
