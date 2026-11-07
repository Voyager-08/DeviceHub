#include "NavigationBar.h"

#include <QButtonGroup>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace devicehub {

NavigationBar::NavigationBar(const QStringList &items, QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("NavigationBar"));
    setFixedWidth(170);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 12, 8, 12);
    m_layout->setSpacing(6);

    auto *brand = new QLabel(QStringLiteral("DeviceHub"), this);
    brand->setObjectName(QStringLiteral("navBrand"));
    QFont brandFont = brand->font();
    brandFont.setBold(true);
    brandFont.setPointSize(brandFont.pointSize() + 4);
    brand->setFont(brandFont);
    brand->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(brand);
    m_layout->addSpacing(8);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    for (int i = 0; i < items.size(); ++i) {
        auto *button = new QPushButton(items.at(i), this);
        button->setCheckable(true);
        button->setAutoDefault(false);
        button->setObjectName(QStringLiteral("NavigationButton"));
        m_group->addButton(button, i);
        m_layout->addWidget(button);
        m_buttons.append(button);
    }

    m_layout->addStretch();

    connect(m_group, &QButtonGroup::idClicked,
            this, &NavigationBar::currentIndexChanged);

    if (!m_buttons.isEmpty())
        setCurrentIndex(0);
}

NavigationBar::~NavigationBar() = default;

int NavigationBar::currentIndex() const
{
    return m_group ? m_group->checkedId() : -1;
}

int NavigationBar::count() const
{
    return m_buttons.size();
}

void NavigationBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_buttons.size())
        return;

    QPushButton *button = m_buttons.at(index);
    const int previous = currentIndex();
    if (button->isChecked())
        return;

    button->setChecked(true);
    if (previous != index)
        emit currentIndexChanged(index);
}

} // namespace devicehub