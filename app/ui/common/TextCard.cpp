#include "TextCard.h"

#include <QLabel>
#include <QVBoxLayout>

#include <utility>

namespace devicehub {

TextCard::TextCard(DashboardItemSpec spec, QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    m_textLabel = new QLabel(QStringLiteral("--"), this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    contentLayout()->addWidget(m_textLabel, 1);
}

void TextCard::setValue(const QVariant &value)
{
    if (!value.isValid()) {
        m_textLabel->setText(QStringLiteral("--"));
        return;
    }
    m_textLabel->setText(value.toString());
}

} // namespace devicehub
