#pragma once

#include "DashboardCard.h"

class QLabel;

namespace devicehub {

// 文本卡:任意文本,自动换行
class TextCard : public DashboardCard
{
    Q_OBJECT

public:
    explicit TextCard(DashboardItemSpec spec, QWidget *parent = nullptr);

    void setValue(const QVariant &value) override;

private:
    QLabel *m_textLabel = nullptr;
};

} // namespace devicehub
