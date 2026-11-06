#pragma once

#include "DashboardCard.h"

class QLabel;

namespace devicehub {

// 状态卡:正常 / 警告 / 错误 / 未知
class StatusCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit StatusCard(DashboardItemSpec spec, QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;

private:
    QLabel *m_iconLabel = nullptr;
    QLabel *m_textLabel = nullptr;
};

} // namespace devicehub
