#include "ValueCard.h"

#include <QLabel>
#include <QVBoxLayout>

#include <cmath>
#include <utility>

namespace devicehub {

ValueCard::ValueCard(DashboardItemSpec spec, QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    m_valueLabel = new QLabel(QStringLiteral("--"), this);
    m_valueLabel->setAlignment(Qt::AlignCenter);

    QFont valueFont = m_valueLabel->font();
    valueFont.setBold(true);
    valueFont.setPointSize(valueFont.pointSize() + 16);
    m_valueLabel->setFont(valueFont);

    contentLayout()->addWidget(m_valueLabel, 1);
}

void ValueCard::setValue(const QVariant &value)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!ok) {
        m_valueLabel->setText(QStringLiteral("--"));
        return;
    }

    QString text;
    if (std::abs(number - std::round(number)) < 0.05)
        text = QString::number(std::llround(number));
    else
        text = QString::number(number, 'f', 1);

    if (!spec().unit.isEmpty())
        text += QStringLiteral(" ") + spec().unit;

    m_valueLabel->setText(text);
}

} // namespace devicehub
