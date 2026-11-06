#pragma once

#include "devicehub/Types.h"

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

namespace devicehub {

class DeviceManager;
class IDevice;

// 轻量会话管理:保证同一设备只有一份运行实例,供设备页与仪表盘共享
class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(DeviceManager *deviceManager,
                            QObject *parent = nullptr);
    ~SessionManager() override;

    IDevice *device(const QString &id) const;
    QStringList deviceIds() const;

    // 创建设备实例(尚未连接)
    IDevice *ensureDevice(const QString &id, QString *errorMessage = nullptr);
    // 创建设备并异步连接,结果通过 deviceStateChanged 返回
    IDevice *openDevice(const QString &id, QString *errorMessage = nullptr);

    void connectDevice(const QString &id);
    void disconnectDevice(const QString &id);
    // 断开并释放设备实例
    void closeDevice(const QString &id);
    void closeAll();

signals:
    void deviceOpened(const QString &id);
    void deviceClosed(const QString &id);
    void deviceStateChanged(const QString &id, devicehub::ConnectionState state);
    void deviceError(const QString &id, const devicehub::ErrorInfo &error);
    void deviceDashboardValueChanged(const QString &id,
                                     const QString &itemId,
                                     const QVariant &value);
    void deviceDashboardItemsChanged(const QString &id);

private:
    DeviceManager *m_deviceManager = nullptr;
    QHash<QString, IDevice *> m_devices;
};

} // namespace devicehub
