#include "CustomCard.h"

#include <QLabel>
#include <QVBoxLayout>

#include <utility>

namespace devicehub {

CustomCard::CustomCard(DashboardItemSpec spec,
                       QWidget *pluginWidget,
                       QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    if (pluginWidget) {
        contentLayout()->addWidget(pluginWidget, 1);
    } else {
        auto *label = new QLabel(QStringLiteral("--"), this);
        label->setAlignment(Qt::AlignCenter);
        contentLayout()->addWidget(label, 1);
    }
}

void CustomCard::setValue(const QVariant &value)
{
    Q_UNUSED(value)
}

} // namespace devicehub
