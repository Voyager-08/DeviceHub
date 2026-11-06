#pragma once

#include "devicehub/Types.h"

#include <QHash>
#include <QIcon>
#include <QWidget>

class QLabel;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace devicehub {

class DeviceManager;
class IDevice;
class PluginManager;
class SessionManager;

// 设备页:设备列表 + 插件提供的设备界面
// 说明:本步尚未引入 SessionManager,页面内临时持有一台设备的实例
class DevicesPage : public QWidget
{
    Q_OBJECT

public:
    explicit DevicesPage(PluginManager *pluginManager,
                         DeviceManager *deviceManager,
                         SessionManager *sessionManager,
                         QWidget *parent = nullptr);
    ~DevicesPage() override;

    void refresh();

    // 直接打开某台设备(供其他页面或测试调用)
    void openDevice(const QString &id);

private slots:
    void onNewDevice();
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onContextMenu(const QPoint &pos);

private:
    void rebuildTree();
    void showDevice(const QString &id);
    IDevice *ensureDevice(const QString &id);
    QWidget *ensureContent(const QString &id);
    void updateItemState(const QString &id);
    void removeLiveDevice(const QString &id);
    QString currentDeviceId() const;
    QString stateText(ConnectionState state) const;
    QIcon stateIcon(ConnectionState state) const;

    PluginManager *m_pluginManager = nullptr;
    DeviceManager *m_deviceManager = nullptr;
    SessionManager *m_sessionManager = nullptr;

    QTreeWidget *m_tree = nullptr;
    QStackedWidget *m_content = nullptr;
    QLabel *m_emptyLabel = nullptr;
    int m_emptyIndex = -1;

    QHash<QString, QWidget *> m_deviceWidgets;
    QHash<QString, QTreeWidgetItem *> m_items;
};

} // namespace devicehub
