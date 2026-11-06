#pragma once

#include <QHash>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QStringList>

#include <memory>
#include <vector>

class QPluginLoader;

namespace devicehub {

class IPlugin;

// 插件在管理器中呈现的状态
enum class PluginState {
    Loaded,     // 已加载
    Disabled,   // 被配置禁用,未加载
    Failed      // 加载失败
};

// 一个插件的完整记录,插件页据此展示
struct PluginInfo {
    QString filePath;
    QString pluginId;
    QString displayName;
    QString version;
    int apiVersion = 0;
    QStringList capabilityIds;
    PluginState state = PluginState::Failed;
    QString errorMessage;
};

// 插件加载失败记录,加载失败的插件不影响主程序启动
struct PluginLoadError {
    QString filePath;
    QString pluginId;
    QString message;
};

// 负责发现、加载插件,检查 API 版本,记录状态与启用状态
class PluginManager : public QObject
{
    Q_OBJECT

public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager() override;

    void setPluginDirectory(const QString &directory);
    QString pluginDirectory() const;

    // 扫描目录并加载全部插件,返回成功加载的数量
    int loadAll();

    // 加载单个插件文件,成功返回 true
    bool loadPlugin(const QString &filePath);

    IPlugin *plugin(const QString &pluginId) const;
    QList<IPlugin *> plugins() const;
    QStringList pluginIds() const;

    QList<PluginLoadError> errors() const { return m_errors; }

    // 所有插件记录(已加载 / 已禁用 / 加载失败)
    QList<PluginInfo> pluginInfos() const { return m_infos; }
    PluginInfo pluginInfo(const QString &pluginId) const;
    QList<PluginInfo> failedPluginInfos() const;
    QList<PluginInfo> disabledPluginInfos() const;

    // 启用状态默认为 true;修改后需重新 loadAll 才生效(不做热加载)
    void setPluginEnabled(const QString &pluginId, bool enabled);
    void setPluginEnabledStates(const QHash<QString, bool> &states);
    bool isPluginEnabled(const QString &pluginId) const;
    QHash<QString, bool> pluginEnabledStates() const { return m_enabledStates; }
    QStringList disabledPluginIds() const;

    // 卸载全部插件,必须在所有插件设备销毁之后调用
    void unloadAll();

    // 当前平台插件库的后缀(dll / so / dylib)
    static QStringList librarySuffixes();

signals:
    void pluginLoaded(const QString &pluginId);
    void pluginLoadFailed(const QString &filePath, const QString &reason);
    void pluginDisabled(const QString &pluginId);

private:
    void recordError(const QString &filePath, const QString &pluginId,
                     const QString &message);
    void recordFailure(const PluginInfo &info, const QString &message);

    QString m_pluginDirectory;
    QHash<QString, IPlugin *> m_plugins;
    std::vector<std::unique_ptr<QPluginLoader>> m_loaders;
    QList<PluginLoadError> m_errors;
    QList<PluginInfo> m_infos;
    QHash<QString, bool> m_enabledStates;
};

} // namespace devicehub

Q_DECLARE_METATYPE(devicehub::PluginState)
Q_DECLARE_METATYPE(devicehub::PluginInfo)
