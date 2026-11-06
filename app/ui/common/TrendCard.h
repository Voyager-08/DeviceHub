#pragma once

#include "DashboardCard.h"

namespace devicehub {

class TrendWidget;

// 趋势卡:保留最近 N 个点的折线图
class TrendCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit TrendCard(DashboardItemSpec spec, QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;

private:
    TrendWidget *m_trend = nullptr;
};

} // namespace devicehub
