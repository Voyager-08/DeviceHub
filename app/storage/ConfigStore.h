#pragma once

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QVariantMap>

namespace devicehub {

// 一台设备的持久化配置
struct DeviceConfig {
    QString id;                 // 唯一标识(UUID)
    QString name;               // 显示名称
    QString pluginId;           // 所属插件
    QString group = QStringLiteral("默认");
    QVariantMap params;         // 连接参数

    bool operator==(const DeviceConfig &other) const
    {
        return id == other.id && name == other.name
            && pluginId == other.pluginId && group == other.group
            && params == other.params;
    }

    bool operator!=(const DeviceConfig &other) const
    {
        return !(*this == other);
    }
};

// 仪表盘卡片布局:设备 id + 项 id + 网格位置
struct DashboardItemLayout {
    QString deviceId;
    QString itemId;
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;
    bool visible = true;

    bool operator==(const DashboardItemLayout &other) const
    {
        return deviceId == other.deviceId && itemId == other.itemId
            && row == other.row && column == other.column
            && rowSpan == other.rowSpan && columnSpan == other.columnSpan
            && visible == other.visible;
    }
};

// 应用级设置
struct AppSettings {
    QString theme = QStringLiteral("system");   // light | dark | system
    QString language = QStringLiteral("zh_CN");

    bool operator==(const AppSettings &other) const
    {
        return theme == other.theme && language == other.language;
    }
};

// 设备列表、仪表盘布局、插件启用状态、应用设置统一持久化到 JSON
class ConfigStore
{
public:
    static constexpr int kFormatVersion = 1;

    ConfigStore();
    explicit ConfigStore(QString filePath);

    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &filePath) { m_filePath = filePath; }

    // 默认路径:<AppConfigLocation>/devices.json
    static QString defaultFilePath();

    // 文件不存在时返回默认值且不视为错误
    QList<DeviceConfig> load(QString *errorMessage = nullptr) const;
    bool save(const QList<DeviceConfig> &devices,
              QString *errorMessage = nullptr) const;

    QList<DashboardItemLayout> loadDashboardLayout(
        QString *errorMessage = nullptr) const;
    bool saveDashboardLayout(const QList<DashboardItemLayout> &layout,
                             QString *errorMessage = nullptr) const;

    QHash<QString, bool> loadPluginEnabledStates(
        QString *errorMessage = nullptr) const;
    bool savePluginEnabledStates(const QHash<QString, bool> &states,
                                 QString *errorMessage = nullptr) const;

    AppSettings loadAppSettings(QString *errorMessage = nullptr) const;
    bool saveAppSettings(const AppSettings &settings,
                         QString *errorMessage = nullptr) const;

private:
    // 读取根对象;文件不存在时 fileExists 为 false 且不报错
    QJsonObject readRoot(QString *errorMessage,
                         bool *fileExists = nullptr) const;
    bool writeRoot(const QJsonObject &root, QString *errorMessage) const;

    QString m_filePath;
};

} // namespace devicehub

Q_DECLARE_METATYPE(devicehub::DeviceConfig)
Q_DECLARE_METATYPE(devicehub::DashboardItemLayout)
Q_DECLARE_METATYPE(devicehub::AppSettings)
