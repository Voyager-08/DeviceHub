#pragma once

#include "devicehub/Dashboard.h"
#include "devicehub/Types.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace devicehub {

class ICapability;

// 设备:持有一个传输层,并对外暴露若干能力
class IDevice : public QObject
{
    Q_OBJECT

public:
    explicit IDevice(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~IDevice() override = default;

    virtual QString deviceId() const = 0;
    virtual QString displayName() const = 0;
    virtual QString pluginId() const = 0;
    virtual ConnectionState state() const = 0;

    // 异步连接,结果通过 stateChanged / errorOccurred 返回
    virtual void connectDevice() = 0;
    virtual void disconnectDevice() = 0;

    virtual QStringList capabilityIds() const = 0;
    virtual ICapability *capability(const QString &id) = 0;

    // 仪表盘:这台设备能提供哪些项(可动态变化),以及当前值
    virtual QList<devicehub::DashboardItemSpec> dashboardItems() const = 0;
    virtual QVariant dashboardValue(const QString &itemId) const = 0;

signals:
    void stateChanged(devicehub::ConnectionState state);
    void errorOccurred(const devicehub::ErrorInfo &error);
    void dashboardValueChanged(const QString &itemId, const QVariant &value);
    void dashboardItemsChanged();                     // 可用项列表变了
};

} // namespace devicehub
