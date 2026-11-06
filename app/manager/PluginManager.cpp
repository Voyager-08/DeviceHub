#include "PluginManager.h"

#include "devicehub/IPlugin.h"
#include "devicehub/Version.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>

namespace devicehub {

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager()
{
    unloadAll();
}

void PluginManager::setPluginDirectory(const QString &directory)
{
    m_pluginDirectory = directory;
}

QString PluginManager::pluginDirectory() const
{
    return m_pluginDirectory;
}

int PluginManager::loadAll()
{
    unloadAll();
    m_errors.clear();

    const QDir dir(m_pluginDirectory);
    if (!dir.exists()) {
        recordError(m_pluginDirectory, {},
                    QStringLiteral("plugin directory does not exist"));
        return 0;
    }

    QStringList nameFilters;
    const QStringList suffixes = librarySuffixes();
    for (const QString &suffix : suffixes)
        nameFilters << QStringLiteral("*.%1").arg(suffix);

    const QFileInfoList entries =
        dir.entryInfoList(nameFilters, QDir::Files, QDir::Name);

    int loaded = 0;
    for (const QFileInfo &entry : entries) {
        if (loadPlugin(entry.absoluteFilePath()))
            ++loaded;
    }
    return loaded;
}

bool PluginManager::loadPlugin(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        recordError(filePath, {}, QStringLiteral("plugin file does not exist"));
        return false;
    }

    auto loader = std::make_unique<QPluginLoader>(filePath);
    QObject *instance = loader->instance();
    if (!instance) {
        PluginInfo info;
        info.filePath = filePath;
        recordFailure(info, loader->errorString());
        return false;
    }

    auto *plugin = qobject_cast<IPlugin *>(instance);
    if (!plugin) {
        PluginInfo info;
        info.filePath = filePath;
        recordFailure(info, QStringLiteral("not a DeviceHub plugin"));
        loader->unload();
        return false;
    }

    PluginInfo info;
    info.filePath = filePath;
    info.pluginId = plugin->pluginId();
    info.displayName = plugin->displayName();
    info.version = plugin->version();
    info.apiVersion = plugin->apiVersion();
    info.capabilityIds = plugin->capabilityIds();

    if (info.pluginId.isEmpty()) {
        recordFailure(info, QStringLiteral("plugin id is empty"));
        loader->unload();
        return false;
    }

    if (info.apiVersion != kApiVersion) {
        recordFailure(info,
                      QStringLiteral("API version mismatch: plugin=%1 host=%2")
                          .arg(info.apiVersion)
                          .arg(kApiVersion));
        loader->unload();
        return false;
    }

    // 已禁用的插件仍然加载一次以读取元信息,随后立即卸载
    if (!isPluginEnabled(info.pluginId)) {
        info.state = PluginState::Disabled;
        m_infos.append(info);
        emit pluginDisabled(info.pluginId);
        loader->unload();
        return false;
    }

    if (m_plugins.contains(info.pluginId)) {
        recordFailure(info, QStringLiteral("duplicate plugin id"));
        loader->unload();
        return false;
    }

    info.state = PluginState::Loaded;
    m_infos.append(info);
    m_plugins.insert(info.pluginId, plugin);
    m_loaders.push_back(std::move(loader));

    emit pluginLoaded(info.pluginId);
    return true;
}

IPlugin *PluginManager::plugin(const QString &pluginId) const
{
    return m_plugins.value(pluginId, nullptr);
}

QList<IPlugin *> PluginManager::plugins() const
{
    return m_plugins.values();
}

QStringList PluginManager::pluginIds() const
{
    return m_plugins.keys();
}

PluginInfo PluginManager::pluginInfo(const QString &pluginId) const
{
    for (const PluginInfo &info : m_infos) {
        if (info.pluginId == pluginId)
            return info;
    }
    return {};
}

QList<PluginInfo> PluginManager::failedPluginInfos() const
{
    QList<PluginInfo> result;
    for (const PluginInfo &info : m_infos) {
        if (info.state == PluginState::Failed)
            result.append(info);
    }
    return result;
}

QList<PluginInfo> PluginManager::disabledPluginInfos() const
{
    QList<PluginInfo> result;
    for (const PluginInfo &info : m_infos) {
        if (info.state == PluginState::Disabled)
            result.append(info);
    }
    return result;
}

void PluginManager::setPluginEnabled(const QString &pluginId, bool enabled)
{
    m_enabledStates.insert(pluginId, enabled);
}

void PluginManager::setPluginEnabledStates(const QHash<QString, bool> &states)
{
    m_enabledStates = states;
}

bool PluginManager::isPluginEnabled(const QString &pluginId) const
{
    return m_enabledStates.value(pluginId, true);
}

QStringList PluginManager::disabledPluginIds() const
{
    QStringList result;
    for (auto it = m_enabledStates.constBegin();
         it != m_enabledStates.constEnd(); ++it) {
        if (!it.value())
            result.append(it.key());
    }
    return result;
}

void PluginManager::unloadAll()
{
    m_plugins.clear();

    for (auto &loader : m_loaders) {
        if (loader && loader->isLoaded())
            loader->unload();
    }
    m_loaders.clear();
    m_infos.clear();
}

QStringList PluginManager::librarySuffixes()
{
#if defined(Q_OS_WIN)
    return {QStringLiteral("dll")};
#elif defined(Q_OS_MACOS)
    return {QStringLiteral("dylib")};
#else
    return {QStringLiteral("so")};
#endif
}

void PluginManager::recordError(const QString &filePath,
                                const QString &pluginId,
                                const QString &message)
{
    m_errors.append(PluginLoadError{filePath, pluginId, message});
    qWarning().noquote() << "Plugin load failed:" << filePath << "-" << message;
    emit pluginLoadFailed(filePath, message);
}

void PluginManager::recordFailure(const PluginInfo &info, const QString &message)
{
    PluginInfo failed = info;
    failed.state = PluginState::Failed;
    failed.errorMessage = message;
    m_infos.append(failed);
    recordError(failed.filePath, failed.pluginId, message);
}

} // namespace devicehub
