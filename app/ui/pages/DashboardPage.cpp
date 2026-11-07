#include "DashboardPage.h"

#include "DashboardEditDialog.h"

#include "devicehub/IDevice.h"
#include "devicehub/IPlugin.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "manager/SessionManager.h"
#include "ui/common/CustomCard.h"
#include "ui/common/GaugeCard.h"
#include "ui/common/StatusCard.h"
#include "ui/common/TextCard.h"
#include "ui/common/TrendCard.h"
#include "ui/common/ValueCard.h"

#include <QDebug>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace devicehub {

DashboardPage::DashboardPage(PluginManager *pluginManager,
                             DeviceManager *deviceManager,
                             SessionManager *sessionManager,
                             QWidget *parent)
    : QWidget(parent)
    , m_pluginManager(pluginManager)
    , m_deviceManager(deviceManager)
    , m_sessionManager(sessionManager)
{
    if (m_deviceManager)
        m_store = &m_deviceManager->configStore();

    auto *root = new QVBoxLayout(this);

    auto *header = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("仪表盘"), this);
    title->setObjectName(QStringLiteral("pageTitle"));

    auto *editButton = new QPushButton(QStringLiteral("编辑仪表盘"), this);
    editButton->setObjectName(QStringLiteral("pageAction"));
    header->addWidget(title);
    header->addStretch();
    header->addWidget(editButton);
    root->addLayout(header);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_contentWidget = new QWidget(scroll);
    auto *contentLayout = new QVBoxLayout(m_contentWidget);

    m_grid = new QGridLayout();
    m_grid->setHorizontalSpacing(12);
    m_grid->setVerticalSpacing(12);
    contentLayout->addLayout(m_grid);

    m_emptyLabel = new QLabel(
        QStringLiteral("暂无仪表盘卡片  请先在设备页连接一台设备"), m_contentWidget);
    m_emptyLabel->setObjectName(QStringLiteral("pageEmptyHint"));
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);
    contentLayout->addWidget(m_emptyLabel, 1);
    contentLayout->addStretch();

    scroll->setWidget(m_contentWidget);
    root->addWidget(scroll, 1);

    m_valueTimer = new QTimer(this);
    m_valueTimer->setSingleShot(true);
    m_valueTimer->setInterval(200);
    connect(m_valueTimer, &QTimer::timeout, this, &DashboardPage::flushValues);

    connect(editButton, &QPushButton::clicked,
            this, &DashboardPage::onEditDashboard);

    if (m_sessionManager) {
        connect(m_sessionManager, &SessionManager::deviceClosed,
                this, &DashboardPage::onDeviceClosed);
        connect(m_sessionManager, &SessionManager::deviceStateChanged,
                this, &DashboardPage::onDeviceStateChanged);
        connect(m_sessionManager, &SessionManager::deviceDashboardValueChanged,
                this, &DashboardPage::onDeviceDashboardValueChanged);
        connect(m_sessionManager, &SessionManager::deviceDashboardItemsChanged,
                this, &DashboardPage::onDeviceDashboardItemsChanged);
    }

    if (m_store) {
        QString error;
        const QList<DashboardItemLayout> saved =
            m_store->loadDashboardLayout(&error);
        if (!error.isEmpty())
            qWarning().noquote() << "Failed to load dashboard layout:" << error;

        for (const DashboardItemLayout &entry : saved)
            m_layout.insert(keyFor(entry.deviceId, entry.itemId), entry);
    }

    refresh();
}

DashboardPage::~DashboardPage() = default;

QString DashboardPage::keyFor(const QString &deviceId, const QString &itemId)
{
    return deviceId + QLatin1Char('/') + itemId;
}

void DashboardPage::refresh()
{
    if (m_sessionManager) {
        const QStringList deviceIds = m_sessionManager->deviceIds();
        for (const QString &deviceId : deviceIds) {
            IDevice *device = m_sessionManager->device(deviceId);
            if (!device)
                continue;

            const bool online = device->state() == ConnectionState::Connected;
            setDeviceCardsOnline(deviceId, online);
            if (online)
                refreshDevice(deviceId);
        }
    }

    for (auto it = m_cards.begin(); it != m_cards.end(); ++it) {
        const DashboardItemSpec spec = m_specs.value(it.key());
        if (spec.kind == DashboardKind::Custom)
            continue;

        IDevice *device =
            m_sessionManager ? m_sessionManager->device(m_specDevice.value(it.key()))
                             : nullptr;
        if (!device)
            continue;

        const QVariant value = device->dashboardValue(spec.id);
        if (value.isValid())
            it.value()->setValue(value);
    }

    relayout();
}

void DashboardPage::onDeviceClosed(const QString &deviceId)
{
    removeDeviceCards(deviceId);
    relayout();
}

void DashboardPage::onDeviceStateChanged(const QString &deviceId,
                                         ConnectionState state)
{
    if (state == ConnectionState::Connected) {
        setDeviceCardsOnline(deviceId, true);
        refreshDevice(deviceId);
    } else {
        setDeviceCardsOnline(deviceId, false);
    }
}

void DashboardPage::onDeviceDashboardValueChanged(const QString &deviceId,
                                                  const QString &itemId,
                                                  const QVariant &value)
{
    // 离线时保留最后一次的值,不接收新数据
    IDevice *device =
        m_sessionManager ? m_sessionManager->device(deviceId) : nullptr;
    if (!device || device->state() != ConnectionState::Connected)
        return;

    m_pendingValues.insert(keyFor(deviceId, itemId), value);
    if (!m_valueTimer->isActive())
        m_valueTimer->start();
}

void DashboardPage::onDeviceDashboardItemsChanged(const QString &deviceId)
{
    if (m_sessionManager && m_sessionManager->device(deviceId))
        refreshDevice(deviceId);
}

void DashboardPage::flushValues()
{
    const QHash<QString, QVariant> pending = m_pendingValues;
    m_pendingValues.clear();

    for (auto it = pending.constBegin(); it != pending.constEnd(); ++it) {
        DashboardCard *card = m_cards.value(it.key(), nullptr);
        if (!card)
            continue;

        const DashboardItemSpec spec = m_specs.value(it.key());
        if (spec.kind == DashboardKind::Custom)
            continue;

        card->setValue(it.value());
    }
}

void DashboardPage::refreshDevice(const QString &deviceId)
{
    if (!m_sessionManager)
        return;

    IDevice *device = m_sessionManager->device(deviceId);
    if (!device || device->state() != ConnectionState::Connected)
        return;

    const QList<DashboardItemSpec> specs = device->dashboardItems();
    QSet<QString> newKeys;

    for (const DashboardItemSpec &spec : specs) {
        const QString key = keyFor(deviceId, spec.id);
        newKeys.insert(key);
        m_specs.insert(key, spec);
        m_specDevice.insert(key, deviceId);
        ensureLayoutEntry(key, spec);

        const auto layoutIt = m_layout.constFind(key);
        if (layoutIt != m_layout.constEnd() && layoutIt->visible)
            ensureCard(key);
    }

    QStringList staleKeys;
    for (auto it = m_specs.constBegin(); it != m_specs.constEnd(); ++it) {
        if (m_specDevice.value(it.key()) == deviceId
            && !newKeys.contains(it.key())) {
            staleKeys.append(it.key());
        }
    }

    for (const QString &key : staleKeys) {
        removeCardWidget(key);
        m_specs.remove(key);
        m_specDevice.remove(key);
        m_layout.remove(key);
        m_layoutDirty = true;
    }

    for (const QString &key : newKeys) {
        DashboardCard *card = m_cards.value(key, nullptr);
        const DashboardItemSpec spec = m_specs.value(key);
        if (!card || spec.kind == DashboardKind::Custom)
            continue;

        const QVariant value = device->dashboardValue(spec.id);
        if (value.isValid())
            card->setValue(value);
    }

    relayout();
    if (m_layoutDirty)
        saveLayout();
}

void DashboardPage::removeDeviceCards(const QString &deviceId)
{
    QStringList keys;
    for (auto it = m_specs.constBegin(); it != m_specs.constEnd(); ++it) {
        if (m_specDevice.value(it.key()) == deviceId)
            keys.append(it.key());
    }

    for (const QString &key : keys) {
        removeCardWidget(key);
        m_specs.remove(key);
        m_specDevice.remove(key);
    }
}

void DashboardPage::ensureCard(const QString &key)
{
    if (m_cards.contains(key))
        return;

    const auto layoutIt = m_layout.constFind(key);
    if (layoutIt == m_layout.constEnd() || !layoutIt->visible)
        return;

    createCard(key);
}

DashboardCard *DashboardPage::createCard(const QString &key)
{
    if (!m_specs.contains(key))
        return nullptr;

    const DashboardItemSpec spec = m_specs.value(key);
    const QString deviceId = m_specDevice.value(key);
    IDevice *device =
        m_sessionManager ? m_sessionManager->device(deviceId) : nullptr;
    if (!device)
        return nullptr;

    IPlugin *plugin =
        m_pluginManager ? m_pluginManager->plugin(device->pluginId()) : nullptr;

    DashboardCard *card = nullptr;

    switch (spec.kind) {
    case DashboardKind::Value:
        card = new ValueCard(spec, m_contentWidget);
        break;
    case DashboardKind::Gauge:
        card = new GaugeCard(spec, m_contentWidget);
        break;
    case DashboardKind::Trend:
        card = new TrendCard(spec, m_contentWidget);
        break;
    case DashboardKind::Status:
        card = new StatusCard(spec, m_contentWidget);
        break;
    case DashboardKind::Text:
        card = new TextCard(spec, m_contentWidget);
        break;
    case DashboardKind::Custom: {
        QWidget *custom = plugin
            ? plugin->createDashboardWidget(spec.id, device, m_contentWidget)
            : nullptr;
        if (custom)
            card = new CustomCard(spec, custom, m_contentWidget);
        else
            card = new TextCard(spec, m_contentWidget);
        break;
    }
    }

    if (!card)
        return nullptr;

    connect(card, &DashboardCard::removeRequested,
            this, &DashboardPage::hideCard);
    card->setOnline(device->state() == ConnectionState::Connected);

    if (spec.kind != DashboardKind::Custom) {
        const QVariant value = device->dashboardValue(spec.id);
        if (value.isValid())
            card->setValue(value);
    }

    m_cards.insert(key, card);
    return card;
}

void DashboardPage::removeCardWidget(const QString &key)
{
    DashboardCard *card = m_cards.take(key);
    if (!card)
        return;

    m_grid->removeWidget(card);
    card->deleteLater();
}

void DashboardPage::hideCard(const QString &key)
{
    auto it = m_layout.find(key);
    if (it != m_layout.end()) {
        it->visible = false;
        m_layoutDirty = true;
    }

    removeCardWidget(key);
    relayout();
    if (m_layoutDirty)
        saveLayout();
}

void DashboardPage::setDeviceCardsOnline(const QString &deviceId, bool online)
{
    for (auto it = m_cards.constBegin(); it != m_cards.constEnd(); ++it) {
        if (m_specDevice.value(it.key()) == deviceId)
            it.value()->setOnline(online);
    }
}

void DashboardPage::ensureLayoutEntry(const QString &key,
                                      const DashboardItemSpec &spec)
{
    if (m_layout.contains(key))
        return;

    DashboardItemLayout entry;
    entry.deviceId = m_specDevice.value(key);
    entry.itemId = spec.id;
    entry.visible = spec.defaultVisible;
    entry.row = -1;
    entry.column = -1;
    entry.rowSpan = qMax(1, spec.defaultSize.height());
    entry.columnSpan = qBound(1, spec.defaultSize.width(), 4);

    m_layout.insert(key, entry);
    m_layoutDirty = true;
}

QStringList DashboardPage::sortedVisibleKeys() const
{
    QStringList keys;
    for (auto it = m_layout.constBegin(); it != m_layout.constEnd(); ++it) {
        if (it->visible && m_specs.contains(it.key()))
            keys.append(it.key());
    }

    std::sort(keys.begin(), keys.end(),
              [this](const QString &a, const QString &b) {
                  const DashboardItemLayout left = m_layout.value(a);
                  const DashboardItemLayout right = m_layout.value(b);
                  const bool leftValid = left.row >= 0 && left.column >= 0;
                  const bool rightValid = right.row >= 0 && right.column >= 0;

                  if (leftValid != rightValid)
                      return leftValid;
                  if (leftValid) {
                      if (left.row != right.row)
                          return left.row < right.row;
                      if (left.column != right.column)
                          return left.column < right.column;
                  }
                  return a < b;
              });

    return keys;
}

bool DashboardPage::cellsAreFree(int row, int column,
                                 int rowSpan, int columnSpan,
                                 const QSet<int> &occupied)
{
    for (int r = row; r < row + rowSpan; ++r) {
        for (int c = column; c < column + columnSpan; ++c) {
            if (occupied.contains(r * 100 + c))
                return false;
        }
    }
    return true;
}

void DashboardPage::markOccupied(QSet<int> &occupied, int row, int column,
                                 int rowSpan, int columnSpan)
{
    for (int r = row; r < row + rowSpan; ++r) {
        for (int c = column; c < column + columnSpan; ++c)
            occupied.insert(r * 100 + c);
    }
}

bool DashboardPage::findFreeSlot(int rowSpan, int columnSpan,
                                 const QSet<int> &occupied,
                                 int *row, int *column)
{
    const int span = qBound(1, columnSpan, 4);
    const int height = qMax(1, rowSpan);

    for (int r = 0; r < 1000; ++r) {
        for (int c = 0; c + span <= 4; ++c) {
            if (cellsAreFree(r, c, height, span, occupied)) {
                if (row)
                    *row = r;
                if (column)
                    *column = c;
                return true;
            }
        }
    }
    return false;
}

void DashboardPage::relayout()
{
    while (QLayoutItem *item = m_grid->takeAt(0))
        delete item;

    const QStringList keys = sortedVisibleKeys();
    QSet<int> occupied;

    for (const QString &key : keys) {
        ensureCard(key);

        DashboardCard *card = m_cards.value(key, nullptr);
        if (!card)
            continue;

        auto it = m_layout.find(key);
        if (it == m_layout.end())
            continue;

        const DashboardItemSpec spec = m_specs.value(key);

        if (it->columnSpan < 1)
            it->columnSpan = qBound(1, spec.defaultSize.width(), 4);
        if (it->rowSpan < 1)
            it->rowSpan = qMax(1, spec.defaultSize.height());

        if (it->row < 0 || it->column < 0
            || !cellsAreFree(it->row, it->column, it->rowSpan,
                             it->columnSpan, occupied)) {
            int row = 0;
            int column = 0;
            if (findFreeSlot(it->rowSpan, it->columnSpan, occupied,
                             &row, &column)) {
                it->row = row;
                it->column = column;
                m_layoutDirty = true;
            }
        }

        markOccupied(occupied, it->row, it->column,
                     it->rowSpan, it->columnSpan);
        m_grid->addWidget(card, it->row, it->column,
                          it->rowSpan, it->columnSpan);
    }

    for (int column = 0; column < 4; ++column)
        m_grid->setColumnStretch(column, 1);

    m_emptyLabel->setVisible(keys.isEmpty());

    if (m_layoutDirty)
        saveLayout();
}

void DashboardPage::saveLayout()
{
    if (!m_store) {
        m_layoutDirty = false;
        return;
    }

    QString error;
    const QList<DashboardItemLayout> layout = m_layout.values();
    if (!m_store->saveDashboardLayout(layout, &error)) {
        qWarning().noquote() << "Failed to save dashboard layout:" << error;
        return;
    }

    m_layoutDirty = false;
}

void DashboardPage::onEditDashboard()
{
    if (!m_sessionManager || !m_deviceManager)
        return;

    QList<DashboardItemEntry> entries;
    for (auto it = m_specs.constBegin(); it != m_specs.constEnd(); ++it) {
        DashboardItemEntry entry;
        entry.key = it.key();
        entry.deviceId = m_specDevice.value(it.key());
        entry.itemId = it.value().id;
        entry.deviceName = m_deviceManager->device(entry.deviceId).name;
        entry.spec = it.value();
        entries.append(entry);
    }

    if (entries.isEmpty()) {
        QMessageBox::information(
            this, QStringLiteral("编辑仪表盘"),
            QStringLiteral("当前没有可用的仪表盘项，请先连接设备。"));
        return;
    }

    DashboardEditDialog dialog(entries, sortedVisibleKeys(), this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QStringList visibleOrder = dialog.visibleKeys();

    QSet<QString> visibleSet;
    for (const QString &key : visibleOrder)
        visibleSet.insert(key);

    for (auto it = m_specs.constBegin(); it != m_specs.constEnd(); ++it) {
        const QString key = it.key();
        auto layoutIt = m_layout.find(key);
        if (layoutIt == m_layout.end()) {
            ensureLayoutEntry(key, it.value());
            layoutIt = m_layout.find(key);
        }

        layoutIt->visible = visibleSet.contains(key);
        if (layoutIt->visible)
            ensureCard(key);
        else
            removeCardWidget(key);
    }

    int row = 0;
    int column = 0;
    int rowHeight = 1;

    for (const QString &key : visibleOrder) {
        auto layoutIt = m_layout.find(key);
        if (layoutIt == m_layout.end())
            continue;

        const DashboardItemSpec spec = m_specs.value(key);
        const int width = qBound(1, spec.defaultSize.width(), 4);
        const int height = qMax(1, spec.defaultSize.height());

        if (column + width > 4) {
            row += rowHeight;
            column = 0;
            rowHeight = 1;
        }

        layoutIt->row = row;
        layoutIt->column = column;
        layoutIt->rowSpan = height;
        layoutIt->columnSpan = width;

        column += width;
        rowHeight = qMax(rowHeight, height);

        if (column >= 4) {
            row += rowHeight;
            column = 0;
            rowHeight = 1;
        }
    }

    m_layoutDirty = true;
    relayout();
}

} // namespace devicehub