#pragma once

#include <QWidget>

namespace devicehub {

// 插件页:第 8 步填充插件列表与启用/禁用
class PluginsPage : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsPage(QWidget *parent = nullptr);
    ~PluginsPage() override;
};

} // namespace devicehub
