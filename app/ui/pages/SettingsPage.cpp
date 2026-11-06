#include "SettingsPage.h"

#include <QLabel>
#include <QVBoxLayout>

namespace devicehub {

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("设置"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 8);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    auto *hint = new QLabel(QStringLiteral("设置项将在第 8 步实现"), this);
    hint->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addStretch();
}

SettingsPage::~SettingsPage() = default;

} // namespace devicehub
