#include "SettingsPage.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFrame>

namespace devicehub {

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(16);

    auto *title = new QLabel(QStringLiteral("设置"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    layout->addWidget(title);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("emptyStateCard"));
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 48, 32, 48);
    cardLayout->setAlignment(Qt::AlignCenter);

    auto *hint = new QLabel(QStringLiteral("主题、语言与其他设置项将在后续步骤实现"), card);
    hint->setObjectName(QStringLiteral("pageEmptyHint"));
    hint->setAlignment(Qt::AlignCenter);
    hint->setWordWrap(true);
    cardLayout->addWidget(hint);

    layout->addWidget(card, 1);
}

SettingsPage::~SettingsPage() = default;

} // namespace devicehub