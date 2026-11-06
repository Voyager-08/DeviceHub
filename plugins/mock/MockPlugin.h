#pragma once

#include "devicehub/IPlugin.h"

#include <QObject>

namespace devicehub::test {

// 开发用假插件:验证插件扫描、设备界面、仪表盘与设置页
class MockPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.devicehub.IPlugin/1.0" FILE "metadata.json")
    Q_INTERFACES(devicehub::IPlugin)

public:
    QString pluginId() const override { return QStringLiteral("mock"); }
    QString displayName() const override { return QStringLiteral("Mock Plugin"); }
    QString version() const override { return QStringLiteral("1.0.0"); }
    int apiVersion() const override;
    QStringList capabilityIds() const override;
    QList<ParamSpec> paramSpecs() const override;

    IDevice *createDevice(const QVariantMap &params, QObject *parent) override;

    QWidget *createDeviceWidget(IDevice *device, QWidget *parent) override;
    QWidget *createDashboardWidget(const QString &itemId,
                                   IDevice *device,
                                   QWidget *parent) override;
    QWidget *createSettingsWidget(QWidget *parent) override;
};

} // namespace devicehub::test
