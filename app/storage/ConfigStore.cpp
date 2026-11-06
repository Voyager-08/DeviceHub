#include "ConfigStore.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QStandardPaths>
#include <QUuid>

#include <utility>

namespace devicehub {

namespace {

void setError(QString *errorMessage, const QString &message)
{
    if (errorMessage)
        *errorMessage = message;
}

} // namespace

ConfigStore::ConfigStore()
    : m_filePath(defaultFilePath())
{
}

ConfigStore::ConfigStore(QString filePath)
    : m_filePath(std::move(filePath))
{
}

QString ConfigStore::defaultFilePath()
{
    const QString dir =
        QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(dir).filePath(QStringLiteral("devices.json"));
}

QJsonObject ConfigStore::readRoot(QString *errorMessage, bool *fileExists) const
{
    if (fileExists)
        *fileExists = false;

    setError(errorMessage, {});

    if (m_filePath.isEmpty()) {
        setError(errorMessage, QStringLiteral("config file path is empty"));
        return {};
    }

    QFile file(m_filePath);
    if (!file.exists())
        return {};

    if (!file.open(QIODevice::ReadOnly)) {
        setError(errorMessage,
                 QStringLiteral("cannot open config file: %1")
                     .arg(file.errorString()));
        return {};
    }

    QJsonParseError parseError{};
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        setError(errorMessage,
                 QStringLiteral("invalid JSON at offset %1: %2")
                     .arg(parseError.offset)
                     .arg(parseError.errorString()));
        return {};
    }

    if (!doc.isObject()) {
        setError(errorMessage, QStringLiteral("config root must be an object"));
        return {};
    }

    const QJsonObject root = doc.object();
    const int version = root.value(QStringLiteral("version")).toInt(kFormatVersion);
    if (version > kFormatVersion) {
        setError(errorMessage,
                 QStringLiteral("unsupported config version: %1").arg(version));
        return {};
    }

    if (fileExists)
        *fileExists = true;
    return root;
}

bool ConfigStore::writeRoot(const QJsonObject &root,
                            QString *errorMessage) const
{
    setError(errorMessage, {});

    if (m_filePath.isEmpty()) {
        setError(errorMessage, QStringLiteral("config file path is empty"));
        return false;
    }

    QJsonObject output = root;
    output.insert(QStringLiteral("version"), kFormatVersion);

    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath())) {
        setError(errorMessage,
                 QStringLiteral("cannot create config directory: %1")
                     .arg(info.absolutePath()));
        return false;
    }

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        setError(errorMessage,
                 QStringLiteral("cannot open config file for writing: %1")
                     .arg(file.errorString()));
        return false;
    }

    if (file.write(QJsonDocument(output).toJson(QJsonDocument::Indented)) < 0) {
        setError(errorMessage,
                 QStringLiteral("cannot write config file: %1")
                     .arg(file.errorString()));
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        setError(errorMessage,
                 QStringLiteral("cannot commit config file: %1")
                     .arg(file.errorString()));
        return false;
    }

    return true;
}

QList<DeviceConfig> ConfigStore::load(QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    const QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty() || !exists)
        return {};

    QList<DeviceConfig> devices;
    const QJsonArray array = root.value(QStringLiteral("devices")).toArray();
    devices.reserve(array.size());

    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject object = value.toObject();

        DeviceConfig config;
        config.id = object.value(QStringLiteral("id")).toString();
        config.name = object.value(QStringLiteral("name")).toString();
        config.pluginId = object.value(QStringLiteral("pluginId")).toString();
        config.group = object.value(QStringLiteral("group")).toString();
        config.params =
            object.value(QStringLiteral("params")).toObject().toVariantMap();

        if (config.id.isEmpty())
            config.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (config.name.isEmpty())
            config.name = config.id;
        if (config.group.isEmpty())
            config.group = QStringLiteral("默认");

        devices.append(config);
    }

    return devices;
}

bool ConfigStore::save(const QList<DeviceConfig> &devices,
                       QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty())
        return false;

    QJsonArray array;
    for (const DeviceConfig &device : devices) {
        QJsonObject object;
        object.insert(QStringLiteral("id"), device.id);
        object.insert(QStringLiteral("name"), device.name);
        object.insert(QStringLiteral("pluginId"), device.pluginId);
        object.insert(QStringLiteral("group"), device.group);
        object.insert(QStringLiteral("params"),
                      QJsonObject::fromVariantMap(device.params));
        array.append(object);
    }

    root.insert(QStringLiteral("devices"), array);
    return writeRoot(root, errorMessage);
}

QList<DashboardItemLayout> ConfigStore::loadDashboardLayout(
    QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    const QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty() || !exists)
        return {};

    QList<DashboardItemLayout> layout;
    const QJsonArray array =
        root.value(QStringLiteral("dashboardLayout")).toArray();
    layout.reserve(array.size());

    for (const QJsonValue &value : array) {
        if (!value.isObject())
            continue;

        const QJsonObject object = value.toObject();
        DashboardItemLayout item;
        item.deviceId = object.value(QStringLiteral("deviceId")).toString();
        item.itemId = object.value(QStringLiteral("itemId")).toString();
        item.row = object.value(QStringLiteral("row")).toInt();
        item.column = object.value(QStringLiteral("column")).toInt();
        item.rowSpan = object.value(QStringLiteral("rowSpan")).toInt(1);
        item.columnSpan = object.value(QStringLiteral("columnSpan")).toInt(1);
        item.visible = object.value(QStringLiteral("visible")).toBool(true);

        if (item.deviceId.isEmpty() || item.itemId.isEmpty())
            continue;

        layout.append(item);
    }

    return layout;
}

bool ConfigStore::saveDashboardLayout(
    const QList<DashboardItemLayout> &layout,
    QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty())
        return false;

    QJsonArray array;
    for (const DashboardItemLayout &item : layout) {
        QJsonObject object;
        object.insert(QStringLiteral("deviceId"), item.deviceId);
        object.insert(QStringLiteral("itemId"), item.itemId);
        object.insert(QStringLiteral("row"), item.row);
        object.insert(QStringLiteral("column"), item.column);
        object.insert(QStringLiteral("rowSpan"), item.rowSpan);
        object.insert(QStringLiteral("columnSpan"), item.columnSpan);
        object.insert(QStringLiteral("visible"), item.visible);
        array.append(object);
    }

    root.insert(QStringLiteral("dashboardLayout"), array);
    return writeRoot(root, errorMessage);
}

QHash<QString, bool> ConfigStore::loadPluginEnabledStates(
    QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    const QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty() || !exists)
        return {};

    QHash<QString, bool> states;
    const QJsonObject object = root.value(QStringLiteral("pluginEnabled")).toObject();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it)
        states.insert(it.key(), it.value().toBool(true));

    return states;
}

bool ConfigStore::savePluginEnabledStates(
    const QHash<QString, bool> &states,
    QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty())
        return false;

    QJsonObject object;
    for (auto it = states.constBegin(); it != states.constEnd(); ++it)
        object.insert(it.key(), it.value());

    root.insert(QStringLiteral("pluginEnabled"), object);
    return writeRoot(root, errorMessage);
}

AppSettings ConfigStore::loadAppSettings(QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    const QJsonObject root = readRoot(error, &exists);

    AppSettings settings;
    if (!error->isEmpty() || !exists)
        return settings;

    const QJsonObject object = root.value(QStringLiteral("settings")).toObject();
    settings.theme =
        object.value(QStringLiteral("theme")).toString(settings.theme);
    settings.language =
        object.value(QStringLiteral("language")).toString(settings.language);
    return settings;
}

bool ConfigStore::saveAppSettings(const AppSettings &settings,
                                  QString *errorMessage) const
{
    QString localError;
    QString *error = errorMessage ? errorMessage : &localError;

    bool exists = false;
    QJsonObject root = readRoot(error, &exists);
    if (!error->isEmpty())
        return false;

    QJsonObject object;
    object.insert(QStringLiteral("theme"), settings.theme);
    object.insert(QStringLiteral("language"), settings.language);

    root.insert(QStringLiteral("settings"), object);
    return writeRoot(root, errorMessage);
}

} // namespace devicehub
