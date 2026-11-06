#include "DashboardCard.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

#include <utility>

namespace devicehub {

DashboardCard::DashboardCard(DashboardItemSpec spec, QWidget *parent)
    : QWidget(parent)
    , m_spec(std::move(spec))
{
    setObjectName(QStringLiteral("DashboardCard"));
    setMinimumSize(160 * qMax(1, m_spec.defaultSize.width()),
                   110 * qMax(1, m_spec.defaultSize.height()));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(6);

    auto *titleRow = new QHBoxLayout();
    titleRow->setSpacing(4);

    auto *title = new QLabel(m_spec.title.isEmpty() ? m_spec.id : m_spec.title, this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_removeButton = new QToolButton(this);
    m_removeButton->setText(QStringLiteral("×"));
    m_removeButton->setToolTip(QStringLiteral("移除卡片"));
    m_removeButton->setVisible(false);
    connect(m_removeButton, &QToolButton::clicked, this,
            [this]() { emit removeRequested(m_spec.id); });

    titleRow->addWidget(title);
    titleRow->addStretch();
    titleRow->addWidget(m_removeButton);
    root->addLayout(titleRow);

    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(4);
    root->addLayout(m_contentLayout, 1);

    m_offlineLabel = new QLabel(QStringLiteral("离线"), this);
    m_offlineLabel->setAlignment(Qt::AlignCenter);
    m_offlineLabel->setVisible(false);
    root->addWidget(m_offlineLabel);
}

DashboardCard::~DashboardCard() = default;

void DashboardCard::setOnline(bool online)
{
    if (m_online == online)
        return;

    m_online = online;
    setEnabled(online);
    m_offlineLabel->setVisible(!online);
    m_removeButton->setVisible(m_editMode);
}

void DashboardCard::setEditMode(bool edit)
{
    m_editMode = edit;
    m_removeButton->setVisible(edit);
}

void DashboardCard::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QPalette pal = palette();
    painter.setPen(QPen(pal.color(QPalette::Mid), 1));
    painter.setBrush(pal.color(QPalette::Base));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

} // namespace devicehub
