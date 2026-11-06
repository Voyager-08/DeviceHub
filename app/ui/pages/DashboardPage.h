#pragma once

#include "devicehub/Dashboard.h"
#include "devicehub/Types.h"
#include "storage/ConfigStore.h"

#include <QHash>
#include <QSet>
#include <QVariant>
#include <QWidget>

class QGridLayout;
class QLabel;
class QTimer;

namespace devicehub {

class DashboardCard;
class DeviceManager;
class IDevice;
class PluginManager;
class SessionManager;

// 仪表盘页:内容完全由设备的 dashboardItems() 声明
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(PluginManager *pluginManager,
                           DeviceManager *deviceManager,
                           SessionManager *sessionManager,
                           QWidget *parent = nullptr);
    ~DashboardPage() override;

    void refresh();

private slots:
    void onDeviceClosed(const QString &deviceId);
    void onDeviceStateChanged(const QString &deviceId, ConnectionState state);
    void onDeviceDashboardValueChanged(const QString &deviceId,
                                       const QString &itemId,
                                       const QVariant &value);
    void onDeviceDashboardItemsChanged(const QString &deviceId);
    void onEditDashboard();
    void flushValues();

private:
    void refreshDevice(const QString &deviceId);
    void removeDeviceCards(const QString &deviceId);
    void ensureCard(const QString &key);
    DashboardCard *createCard(const QString &key);
    void removeCardWidget(const QString &key);
    void hideCard(const QString &key);
    void relayout();
    void saveLayout();
    void setDeviceCardsOnline(const QString &deviceId, bool online);
    void ensureLayoutEntry(const QString &key, const DashboardItemSpec &spec);
    QStringList sortedVisibleKeys() const;

    static QString keyFor(const QString &deviceId, const QString &itemId);
    static bool findFreeSlot(int rowSpan, int columnSpan,
                             const QSet<int> &occupied, int *row, int *column);
    static void markOccupied(QSet<int> &occupied, int row, int column,
                             int rowSpan, int columnSpan);
    static bool cellsAreFree(int row, int column, int rowSpan, int columnSpan,
                             const QSet<int> &occupied);

    PluginManager *m_pluginManager = nullptr;
    DeviceManager *m_deviceManager = nullptr;
    SessionManager *m_sessionManager = nullptr;
    ConfigStore *m_store = nullptr;

    QWidget *m_contentWidget = nullptr;
    QGridLayout *m_grid = nullptr;
    QLabel *m_emptyLabel = nullptr;
    QTimer *m_valueTimer = nullptr;

    QHash<QString, DashboardItemSpec> m_specs;
    QHash<QString, QString> m_specDevice;
    QHash<QString, DashboardCard *> m_cards;
    QHash<QString, DashboardItemLayout> m_layout;
    QHash<QString, QVariant> m_pendingValues;

    bool m_layoutDirty = false;
};

} // namespace devicehub
