#pragma once

#include "DashboardCard.h"

namespace devicehub {

// 自定义卡片:内容由插件提供,值和刷新由插件控件自行处理
class CustomCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit CustomCard(DashboardItemSpec spec,
                        QWidget *pluginWidget,
                        QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;
};

} // namespace devicehub
