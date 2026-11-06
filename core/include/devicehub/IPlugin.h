#pragma once

#include "devicehub/Types.h"

#include <QIcon>
#include <QList>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>
#include <QtPlugin>

namespace devicehub {

class IDevice;

// 插件接口:主程序只通过这个接口与插件通信
class IPlugin
{
public:
    virtual ~IPlugin() = default;

    // 基本信息
    virtual QString pluginId() const = 0;
    virtual QString displayName() const = 0;
    virtual QString version() const = 0;
    virtual int apiVersion() const = 0;
    virtual QStringList capabilityIds() const = 0;
    virtual QList<ParamSpec> paramSpecs() const = 0;
    virtual QIcon icon() const { return {}; }

    // 创建设备
    virtual IDevice *createDevice(const QVariantMap &params, QObject *parent) = 0;

    // 界面入口(都可选;返回 nullptr 表示用主程序的默认实现)
    // 点击设备后显示的设备主界面
    virtual QWidget *createDeviceWidget(IDevice *device, QWidget *parent)
    {
        Q_UNUSED(device)
        Q_UNUSED(parent)
        return nullptr;
    }

    // 仪表盘项 kind 为 Custom 时,由它创建自定义控件
    virtual QWidget *createDashboardWidget(const QString &itemId,
                                           IDevice *device,
                                           QWidget *parent)
    {
        Q_UNUSED(itemId)
        Q_UNUSED(device)
        Q_UNUSED(parent)
        return nullptr;
    }

    // 设置页中属于这个插件的设置界面
    virtual QWidget *createSettingsWidget(QWidget *parent)
    {
        Q_UNUSED(parent)
        return nullptr;
    }

    // 旧版通用面板入口,保留以兼容已有插件
    virtual QWidget *createPanel(const QString &capabilityId, IDevice *device, QWidget *parent)
    {
        Q_UNUSED(capabilityId)
        Q_UNUSED(device)
        Q_UNUSED(parent)
        return nullptr;
    }
};

} // namespace devicehub

Q_DECLARE_INTERFACE(devicehub::IPlugin, "org.devicehub.IPlugin/1.0")
