#pragma once

#include "storage/ConfigStore.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

namespace devicehub {

class IDevice;
class PluginManager;

// 设备配置的增删改查,并按配置通过插件创建设备实例
class DeviceManager : public QObject
{
    Q_OBJECT

public:
    explicit DeviceManager(PluginManager *pluginManager, QObject *parent = nullptr);
    DeviceManager(PluginManager *pluginManager,
                  const QString &configFilePath,
                  QObject *parent = nullptr);
    ~DeviceManager() override;

    PluginManager *pluginManager() const { return m_pluginManager; }

    ConfigStore &configStore() { return m_store; }
    const ConfigStore &configStore() const { return m_store; }

    QString configFilePath() const { return m_store.filePath(); }
    void setConfigFilePath(const QString &filePath) { m_store.setFilePath(filePath); }

    bool load(QString *errorMessage = nullptr);
    bool save(QString *errorMessage = nullptr) const;

    QList<DeviceConfig> devices() const { return m_devices; }
    DeviceConfig device(const QString &id) const;
    bool contains(const QString &id) const;
    QStringList groups() const;

    // 成功返回设备 id(为空时自动生成 UUID),失败返回空字符串
    QString addDevice(const DeviceConfig &config, QString *errorMessage = nullptr);
    bool updateDevice(const DeviceConfig &config, QString *errorMessage = nullptr);
    bool removeDevice(const QString &id, QString *errorMessage = nullptr);

    // 通过插件按配置创建一台设备,所有权交给调用方或 parent
    IDevice *createDevice(const QString &id,
                          QObject *parent = nullptr,
                          QString *errorMessage = nullptr) const;

signals:
    void deviceAdded(const QString &id);
    void deviceUpdated(const QString &id);
    void deviceRemoved(const QString &id);
    void devicesChanged();

private:
    int indexOf(const QString &id) const;

    PluginManager *m_pluginManager = nullptr;
    ConfigStore m_store;
    QList<DeviceConfig> m_devices;
};

} // namespace devicehub
