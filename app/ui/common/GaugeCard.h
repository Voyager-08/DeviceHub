#pragma once

#include "DashboardCard.h"

namespace devicehub {

class GaugeWidget;

// 仪表盘卡:进度环 + 当前值
class GaugeCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit GaugeCard(DashboardItemSpec spec, QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;

private:
    GaugeWidget *m_gauge = nullptr;
};

} // namespace devicehub
