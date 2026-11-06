#include "DeviceManager.h"

#include "PluginManager.h"

#include "devicehub/IDevice.h"
#include "devicehub/IPlugin.h"

#include <QUuid>

namespace devicehub {

namespace {

void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
        *errorMessage = message;
}

} // namespace

DeviceManager::DeviceManager(PluginManager *pluginManager, QObject *parent)
    : DeviceManager(pluginManager, ConfigStore::defaultFilePath(), parent)
{
}

DeviceManager::DeviceManager(PluginManager *pluginManager,
                             const QString &configFilePath,
                             QObject *parent)
    : QObject(parent)
    , m_pluginManager(pluginManager)
    , m_store(configFilePath)
{
}

DeviceManager::~DeviceManager() = default;

bool DeviceManager::load(QString *errorMessage)
{
    QString storeError;
    const QList<DeviceConfig> loaded = m_store.load(&storeError);
    if (!storeError.isEmpty()) {
        setError(errorMessage, storeError);
        return false;
    }

    m_devices = loaded;
    emit devicesChanged();
    return true;
}

bool DeviceManager::save(QString *errorMessage) const
{
    return m_store.save(m_devices, errorMessage);
}

DeviceConfig DeviceManager::device(const QString &id) const
{
    const int index = indexOf(id);
    return index >= 0 ? m_devices.at(index) : DeviceConfig{};
}

bool DeviceManager::contains(const QString &id) const
{
    return indexOf(id) >= 0;
}

QStringList DeviceManager::groups() const
{
    QStringList result;
    for (const DeviceConfig &config : m_devices) {
        if (!config.group.isEmpty() && !result.contains(config.group))
            result.append(config.group);
    }
    return result;
}

QString DeviceManager::addDevice(const DeviceConfig &config, QString *errorMessage)
{
    setError(errorMessage, {});

    if (config.pluginId.isEmpty()) {
        setError(errorMessage, QStringLiteral("plugin id is required"));
        return {};
    }

    DeviceConfig stored = config;
    if (stored.id.isEmpty())
        stored.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (stored.name.isEmpty())
        stored.name = stored.id;
    if (stored.group.isEmpty())
        stored.group = QStringLiteral("默认");

    if (indexOf(stored.id) >= 0) {
        setError(errorMessage,
                 QStringLiteral("device id already exists: %1").arg(stored.id));
        return {};
    }

    m_devices.append(stored);

    QString saveError;
    if (!m_store.save(m_devices, &saveError)) {
        m_devices.removeLast();
        setError(errorMessage, saveError);
        return {};
    }

    emit deviceAdded(stored.id);
    emit devicesChanged();
    return stored.id;
}

bool DeviceManager::updateDevice(const DeviceConfig &config, QString *errorMessage)
{
    setError(errorMessage, {});

    const int index = indexOf(config.id);
    if (index < 0) {
        setError(errorMessage,
                 QStringLiteral("device not found: %1").arg(config.id));
        return false;
    }

    if (config.pluginId.isEmpty()) {
        setError(errorMessage, QStringLiteral("plugin id is required"));
        return false;
    }

    DeviceConfig stored = config;
    if (stored.name.isEmpty())
        stored.name = stored.id;
    if (stored.group.isEmpty())
        stored.group = QStringLiteral("默认");

    const DeviceConfig previous = m_devices.at(index);
    m_devices[index] = stored;

    QString saveError;
    if (!m_store.save(m_devices, &saveError)) {
        m_devices[index] = previous;
        setError(errorMessage, saveError);
        return false;
    }

    emit deviceUpdated(stored.id);
    emit devicesChanged();
    return true;
}

bool DeviceManager::removeDevice(const QString &id, QString *errorMessage)
{
    setError(errorMessage, {});

    const int index = indexOf(id);
    if (index < 0) {
        setError(errorMessage, QStringLiteral("device not found: %1").arg(id));
        return false;
    }

    const DeviceConfig previous = m_devices.at(index);
    m_devices.removeAt(index);

    QString saveError;
    if (!m_store.save(m_devices, &saveError)) {
        m_devices.insert(index, previous);
        setError(errorMessage, saveError);
        return false;
    }

    emit deviceRemoved(id);
    emit devicesChanged();
    return true;
}

IDevice *DeviceManager::createDevice(const QString &id,
                                     QObject *parent,
                                     QString *errorMessage) const
{
    setError(errorMessage, {});

    const int index = indexOf(id);
    if (index < 0) {
        setError(errorMessage, QStringLiteral("device not found: %1").arg(id));
        return nullptr;
    }

    if (!m_pluginManager) {
        setError(errorMessage, QStringLiteral("plugin manager is not available"));
        return nullptr;
    }

    const DeviceConfig &config = m_devices.at(index);
    IPlugin *plugin = m_pluginManager->plugin(config.pluginId);
    if (!plugin) {
        setError(errorMessage,
                 QStringLiteral("plugin not found: %1").arg(config.pluginId));
        return nullptr;
    }

    IDevice *device = plugin->createDevice(config.params, parent);
    if (!device) {
        setError(errorMessage,
                 QStringLiteral("plugin failed to create device: %1")
                     .arg(config.pluginId));
        return nullptr;
    }

    return device;
}

int DeviceManager::indexOf(const QString &id) const
{
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices.at(i).id == id)
            return i;
    }
    return -1;
}

} // namespace devicehub
