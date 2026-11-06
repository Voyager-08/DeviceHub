#include "PluginsPage.h"

#include <QLabel>
#include <QVBoxLayout>

namespace devicehub {

PluginsPage::PluginsPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);

    auto *title = new QLabel(QStringLiteral("插件"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 8);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    auto *hint = new QLabel(QStringLiteral("插件管理将在第 8 步实现"), this);
    hint->setAlignment(Qt::AlignCenter);

    layout->addStretch();
    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addStretch();
}

PluginsPage::~PluginsPage() = default;

} // namespace devicehub
