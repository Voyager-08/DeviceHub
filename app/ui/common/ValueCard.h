#pragma once

#include "DashboardCard.h"

class QLabel;

namespace devicehub {

// 数值卡:大号数字 + 单位
class ValueCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit ValueCard(DashboardItemSpec spec, QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;

private:
    QLabel *m_valueLabel = nullptr;
};

} // namespace devicehub
