#pragma once

#include <QWidget>

namespace devicehub {

// 设置页:第 8 步填充主题、语言与插件设置
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage() override;
};

} // namespace devicehub
