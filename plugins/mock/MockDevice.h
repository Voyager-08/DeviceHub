#pragma once

#include "devicehub/IDevice.h"

#include "MockTransport.h"

class QTimer;

namespace devicehub::test {

class MockTerminal;

// 测试用设备:内部持有 MockTransport,提供 terminal 能力与四个仪表盘项
class MockDevice : public IDevice
{
    Q_OBJECT

public:
    explicit MockDevice(QString id,
                        QString displayName = QString(),
                        QObject *parent = nullptr);
    ~MockDevice() override;

    QString deviceId() const override;
    QString displayName() const override;
    QString pluginId() const override;
    ConnectionState state() const override;

    void connectDevice() override;
    void disconnectDevice() override;

    QStringList capabilityIds() const override;
    ICapability *capability(const QString &id) override;

    QList<DashboardItemSpec> dashboardItems() const override;
    QVariant dashboardValue(const QString &itemId) const override;

    // 保存连接参数,下次 connectDevice() 时使用
    void setConnectionParams(const QVariantMap &params);
    // 兼容旧接口:立即用参数打开连接
    void setParams(const QVariantMap &params);

    MockTransport *transport() const { return m_transport; }

private:
    void refreshDashboardValues();

    QString m_id;
    QString m_displayName;
    MockTransport *m_transport = nullptr;
    MockTerminal *m_terminal = nullptr;
    QTimer *m_dashboardTimer = nullptr;
    QVariantMap m_connectionParams;
    QVariantMap m_dashboardValues;
    double m_phase = 0.0;
    int m_tick = 0;
};

} // namespace devicehub::test
