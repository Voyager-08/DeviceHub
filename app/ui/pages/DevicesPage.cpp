#include "DevicesPage.h"

#include "NewDeviceDialog.h"

#include "devicehub/IDevice.h"
#include "devicehub/IPlugin.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "manager/SessionManager.h"

#include <QAction>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QStyle>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace devicehub {

DevicesPage::DevicesPage(PluginManager *pluginManager,
                         DeviceManager *deviceManager,
                         SessionManager *sessionManager,
                         QWidget *parent)
    : QWidget(parent)
    , m_pluginManager(pluginManager)
    , m_deviceManager(deviceManager)
    , m_sessionManager(sessionManager)
{
    auto *root = new QVBoxLayout(this);

    auto *header = new QHBoxLayout();
    auto *title = new QLabel(QStringLiteral("设备"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 6);
    title->setFont(titleFont);

    auto *newButton = new QPushButton(QStringLiteral("新建设备"), this);
    header->addWidget(title);
    header->addStretch();
    header->addWidget(newButton);
    root->addLayout(header);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_tree = new QTreeWidget(splitter);
    m_tree->setColumnCount(2);
    m_tree->setHeaderLabels({QStringLiteral("设备"), QStringLiteral("状态")});
    m_tree->setRootIsDecorated(true);
    m_tree->setAlternatingRowColors(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setMinimumWidth(280);

    m_content = new QStackedWidget(splitter);
    m_emptyLabel = new QLabel(QStringLiteral("选择左侧设备查看界面"), m_content);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyIndex = m_content->addWidget(m_emptyLabel);

    splitter->addWidget(m_tree);
    splitter->addWidget(m_content);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({340, 800});

    root->addWidget(splitter, 1);

    connect(newButton, &QPushButton::clicked,
            this, &DevicesPage::onNewDevice);
    connect(m_tree, &QTreeWidget::itemClicked,
            this, &DevicesPage::onItemClicked);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &DevicesPage::onContextMenu);

    if (m_sessionManager) {
        connect(m_sessionManager, &SessionManager::deviceStateChanged, this,
                [this](const QString &id, ConnectionState) { updateItemState(id); });
        connect(m_sessionManager, &SessionManager::deviceError, this,
                [this](const QString &id, const ErrorInfo &error) {
                    if (QTreeWidgetItem *item = m_items.value(id, nullptr))
                        item->setToolTip(0, error.message);
                    updateItemState(id);
                });
        connect(m_sessionManager, &SessionManager::deviceClosed, this,
                [this](const QString &id) { updateItemState(id); });
    }

    refresh();
}

DevicesPage::~DevicesPage() = default;

void DevicesPage::refresh()
{
    rebuildTree();
}

void DevicesPage::openDevice(const QString &id)
{
    showDevice(id);
}

void DevicesPage::rebuildTree()
{
    const QString selected = currentDeviceId();

    m_items.clear();
    m_tree->clear();

    if (!m_deviceManager) {
        m_emptyLabel->setText(QStringLiteral("未接入设备管理器"));
        return;
    }

    const QList<DeviceConfig> devices = m_deviceManager->devices();

    for (const QString &group : m_deviceManager->groups()) {
        auto *groupItem = new QTreeWidgetItem(m_tree);
        groupItem->setText(0, group);
        groupItem->setFirstColumnSpanned(true);
        groupItem->setFlags(Qt::ItemIsEnabled);

        QFont groupFont = groupItem->font(0);
        groupFont.setBold(true);
        groupItem->setFont(0, groupFont);
        groupItem->setExpanded(true);

        for (const DeviceConfig &config : devices) {
            if (config.group != group)
                continue;

            auto *item = new QTreeWidgetItem(groupItem);
            item->setText(0, config.name);
            item->setData(0, Qt::UserRole, config.id);
            item->setToolTip(0, config.id);

            QIcon icon;
            if (m_pluginManager) {
                if (IPlugin *plugin = m_pluginManager->plugin(config.pluginId))
                    icon = plugin->icon();
            }
            if (icon.isNull())
                icon = style()->standardIcon(QStyle::SP_ComputerIcon);
            item->setIcon(0, icon);

            m_items.insert(config.id, item);
            updateItemState(config.id);
        }
    }

    if (!selected.isEmpty() && m_items.contains(selected))
        m_tree->setCurrentItem(m_items.value(selected));
}

void DevicesPage::onNewDevice()
{
    if (!m_deviceManager) {
        QMessageBox::warning(this, QStringLiteral("不可用"),
                             QStringLiteral("尚未接入设备管理器。"));
        return;
    }

    NewDeviceDialog dialog(m_pluginManager, this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString error;
    const QString id = m_deviceManager->addDevice(dialog.deviceConfig(), &error);
    if (id.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("无法新建设备"), error);
        return;
    }

    refresh();
    if (QTreeWidgetItem *item = m_items.value(id, nullptr))
        m_tree->setCurrentItem(item);
}

void DevicesPage::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (!item)
        return;

    const QString id = item->data(0, Qt::UserRole).toString();
    if (!id.isEmpty())
        showDevice(id);
}

void DevicesPage::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    if (!item)
        return;

    const QString id = item->data(0, Qt::UserRole).toString();
    if (id.isEmpty())
        return;

    QMenu menu(this);
    QAction *connectAction = menu.addAction(QStringLiteral("连接"));
    QAction *disconnectAction = menu.addAction(QStringLiteral("断开"));
    menu.addSeparator();
    QAction *editAction = menu.addAction(QStringLiteral("编辑"));
    QAction *deleteAction = menu.addAction(QStringLiteral("删除"));

    QAction *chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
    if (!chosen)
        return;

    if (chosen == connectAction) {
        if (m_sessionManager) {
            QString error;
            if (!m_sessionManager->openDevice(id, &error))
                QMessageBox::warning(this, QStringLiteral("无法连接设备"), error);
        }
        return;
    }

    if (chosen == disconnectAction) {
        if (m_sessionManager)
            m_sessionManager->disconnectDevice(id);
        return;
    }

    if (chosen == editAction) {
        const DeviceConfig config = m_deviceManager->device(id);
        if (config.id.isEmpty())
            return;

        NewDeviceDialog dialog(m_pluginManager, config, this);
        if (dialog.exec() != QDialog::Accepted)
            return;

        QString error;
        if (!m_deviceManager->updateDevice(dialog.deviceConfig(), &error)) {
            QMessageBox::warning(this, QStringLiteral("无法保存设备"), error);
            return;
        }

        removeLiveDevice(id);
        refresh();
        return;
    }

    if (chosen == deleteAction) {
        const auto answer = QMessageBox::question(
            this, QStringLiteral("删除设备"),
            QStringLiteral("确定删除设备「%1」吗？").arg(item->text(0)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes)
            return;

        removeLiveDevice(id);

        QString error;
        if (!m_deviceManager->removeDevice(id, &error)) {
            QMessageBox::warning(this, QStringLiteral("无法删除设备"), error);
            return;
        }
        refresh();
    }
}

void DevicesPage::showDevice(const QString &id)
{
    if (!m_deviceManager || !m_deviceManager->contains(id))
        return;

    if (QWidget *widget = ensureContent(id))
        m_content->setCurrentWidget(widget);
}

IDevice *DevicesPage::ensureDevice(const QString &id)
{
    if (!m_sessionManager) {
        QMessageBox::warning(this, QStringLiteral("无法创建设备"),
                             QStringLiteral("尚未接入会话管理器。"));
        return nullptr;
    }

    QString error;
    IDevice *device = m_sessionManager->ensureDevice(id, &error);
    if (!device)
        QMessageBox::warning(this, QStringLiteral("无法创建设备"), error);
    return device;
}

QWidget *DevicesPage::ensureContent(const QString &id)
{
    if (QWidget *existing = m_deviceWidgets.value(id, nullptr))
        return existing;

    if (!m_sessionManager || !m_deviceManager)
        return nullptr;

    QString openError;
    IDevice *device = m_sessionManager->openDevice(id, &openError);
    if (!device) {
        QMessageBox::warning(this, QStringLiteral("无法打开设备"), openError);
        return nullptr;
    }

    const DeviceConfig config = m_deviceManager->device(id);

    QWidget *widget = nullptr;
    if (m_pluginManager) {
        if (IPlugin *plugin = m_pluginManager->plugin(config.pluginId))
            widget = plugin->createDeviceWidget(device, m_content);
    }

    if (!widget) {
        auto *label = new QLabel(m_content);
        label->setAlignment(Qt::AlignCenter);
        if (device->capabilityIds().contains(QStringLiteral("terminal"))) {
            label->setText(
                QStringLiteral("该设备暂无界面\n（通用终端面板将在 SSH 步骤实现）"));
        } else {
            label->setText(QStringLiteral("该设备暂无界面"));
        }
        widget = label;
    }

    m_content->addWidget(widget);
    m_deviceWidgets.insert(id, widget);
    return widget;
}

void DevicesPage::updateItemState(const QString &id)
{
    QTreeWidgetItem *item = m_items.value(id, nullptr);
    if (!item)
        return;

    ConnectionState state = ConnectionState::Disconnected;
    if (IDevice *device = m_sessionManager ? m_sessionManager->device(id) : nullptr)
        state = device->state();

    item->setText(1, stateText(state));
    item->setIcon(1, stateIcon(state));
}

void DevicesPage::removeLiveDevice(const QString &id)
{
    if (QWidget *widget = m_deviceWidgets.take(id)) {
        m_content->removeWidget(widget);
        widget->deleteLater();
    }

    if (m_sessionManager)
        m_sessionManager->closeDevice(id);

    if (m_content->currentIndex() != m_emptyIndex)
        m_content->setCurrentIndex(m_emptyIndex);
}

QString DevicesPage::currentDeviceId() const
{
    QTreeWidgetItem *item = m_tree ? m_tree->currentItem() : nullptr;
    return item ? item->data(0, Qt::UserRole).toString() : QString();
}

QString DevicesPage::stateText(ConnectionState state) const
{
    switch (state) {
    case ConnectionState::Disconnected:
        return QStringLiteral("未连接");
    case ConnectionState::Connecting:
        return QStringLiteral("连接中");
    case ConnectionState::Connected:
        return QStringLiteral("已连接");
    case ConnectionState::Reconnecting:
        return QStringLiteral("重连中");
    case ConnectionState::Error:
        return QStringLiteral("错误");
    }
    return QStringLiteral("未知");
}

QIcon DevicesPage::stateIcon(ConnectionState state) const
{
    QStyle *style = this->style();
    switch (state) {
    case ConnectionState::Connected:
        return style->standardIcon(QStyle::SP_DialogApplyButton);
    case ConnectionState::Connecting:
    case ConnectionState::Reconnecting:
        return style->standardIcon(QStyle::SP_BrowserReload);
    case ConnectionState::Error:
        return style->standardIcon(QStyle::SP_MessageBoxCritical);
    case ConnectionState::Disconnected:
        break;
    }
    return style->standardIcon(QStyle::SP_DialogCancelButton);
}

} // namespace devicehub
