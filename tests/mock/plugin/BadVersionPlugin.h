#pragma once

#include "devicehub/IPlugin.h"

#include <QObject>

namespace devicehub::test {

// 测试用插件:故意上报不兼容的 API 版本
class BadVersionPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.devicehub.IPlugin/1.0" FILE "metadata_bad.json")
    Q_INTERFACES(devicehub::IPlugin)

public:
    QString pluginId() const override { return QStringLiteral("mock_bad"); }
    QString displayName() const override { return QStringLiteral("Bad Version Plugin"); }
    QString version() const override { return QStringLiteral("1.0.0"); }
    int apiVersion() const override { return 999; }
    QStringList capabilityIds() const override { return {}; }
    QList<ParamSpec> paramSpecs() const override { return {}; }

    IDevice *createDevice(const QVariantMap &params, QObject *parent) override
    {
        Q_UNUSED(params)
        Q_UNUSED(parent)
        return nullptr;
    }
};

} // namespace devicehub::test
