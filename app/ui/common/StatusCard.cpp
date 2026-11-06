#include "StatusCard.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

#include <utility>

namespace devicehub {

namespace {

enum class StatusLevel { Ok, Warning, Error, Unknown };

StatusLevel levelFromValue(const QVariant &value)
{
    if (!value.isValid())
        return StatusLevel::Unknown;

    if (value.typeId() == QMetaType::Bool)
        return value.toBool() ? StatusLevel::Ok : StatusLevel::Error;

    bool isNumber = false;
    const double number = value.toDouble(&isNumber);
    if (isNumber) {
        if (number <= 0.0)
            return StatusLevel::Error;
        if (number >= 2.0)
            return StatusLevel::Warning;
        return StatusLevel::Ok;
    }

    const QString text = value.toString().trimmed().toLower();
    if (text == QStringLiteral("ok") || text == QStringLiteral("normal")
        || text == QStringLiteral("online") || text == QStringLiteral("good")
        || text == QStringLiteral("up") || text == QStringLiteral("true")
        || text == QStringLiteral("1")) {
        return StatusLevel::Ok;
    }
    if (text == QStringLiteral("warn") || text == QStringLiteral("warning")
        || text == QStringLiteral("degraded")) {
        return StatusLevel::Warning;
    }
    if (text == QStringLiteral("error") || text == QStringLiteral("fail")
        || text == QStringLiteral("fault") || text == QStringLiteral("offline")
        || text == QStringLiteral("down") || text == QStringLiteral("critical")
        || text == QStringLiteral("0")) {
        return StatusLevel::Error;
    }

    return StatusLevel::Unknown;
}

} // namespace

StatusCard::StatusCard(DashboardItemSpec spec, QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    auto *row = new QHBoxLayout();
    row->setSpacing(8);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    m_textLabel = new QLabel(QStringLiteral("未知"), this);
    QFont textFont = m_textLabel->font();
    textFont.setBold(true);
    textFont.setPointSize(textFont.pointSize() + 2);
    m_textLabel->setFont(textFont);

    row->addWidget(m_iconLabel);
    row->addWidget(m_textLabel, 1);

    contentLayout()->addStretch();
    contentLayout()->addLayout(row);
    contentLayout()->addStretch();
}

void StatusCard::setValue(const QVariant &value)
{
    const StatusLevel level = levelFromValue(value);

    QStyle *style = this->style();
    QIcon icon;
    QString text;

    switch (level) {
    case StatusLevel::Ok:
        icon = style->standardIcon(QStyle::SP_DialogApplyButton);
        text = QStringLiteral("正常");
        break;
    case StatusLevel::Warning:
        icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
        text = QStringLiteral("警告");
        break;
    case StatusLevel::Error:
        icon = style->standardIcon(QStyle::SP_MessageBoxCritical);
        text = QStringLiteral("错误");
        break;
    case StatusLevel::Unknown:
        text = QStringLiteral("--");
        break;
    }

    m_iconLabel->setPixmap(icon.pixmap(28, 28));
    m_iconLabel->setVisible(!icon.isNull());
    m_textLabel->setText(text);
}

} // namespace devicehub
