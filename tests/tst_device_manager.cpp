#include "devicehub/IDevice.h"
#include "devicehub/Types.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "storage/ConfigStore.h"

#include <QDir>
#include <QFile>
#include <QScopedPointer>
#include <QTemporaryDir>
#include <QtTest>

using namespace devicehub;

class DeviceManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void configStoreRoundTrip();
    void configStoreMissingFile();
    void configStoreMalformedJson();
    void configStoreDashboardLayout();
    void configStorePluginStatesAndSettings();

    void deviceManagerCrud();
    void deviceManagerRejectsDuplicateId();
    void deviceManagerCreatesDevice();

private:
    static QString makeTempDirPath();
};

void DeviceManagerTests::initTestCase()
{
    registerMetaTypes();
}

QString DeviceManagerTests::makeTempDirPath()
{
    const QString base = QStringLiteral(DEVICEHUB_TEST_TEMP_DIR);
    QDir().mkpath(base);
    return base + QStringLiteral("/devicehub-XXXXXX");
}

void DeviceManagerTests::configStoreRoundTrip()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("devices.json"));
    ConfigStore store(path);

    DeviceConfig first;
    first.id = QStringLiteral("dev-1");
    first.name = QStringLiteral("Device 1");
    first.pluginId = QStringLiteral("ssh");
    first.group = QStringLiteral("Lab");
    first.params.insert(QStringLiteral("host"), QStringLiteral("192.168.50.101"));
    first.params.insert(QStringLiteral("port"), 22);

    DeviceConfig second;
    second.id = QStringLiteral("dev-2");
    second.name = QStringLiteral("Device 2");
    second.pluginId = QStringLiteral("mock");
    second.group = QStringLiteral("Office");

    QString error;
    QVERIFY2(store.save({first, second}, &error), qPrintable(error));
    QVERIFY(QFile::exists(path));

    const QList<DeviceConfig> loaded = store.load(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loaded.size(), 2);
    QCOMPARE(loaded.at(0).id, first.id);
    QCOMPARE(loaded.at(0).name, first.name);
    QCOMPARE(loaded.at(0).pluginId, first.pluginId);
    QCOMPARE(loaded.at(0).group, first.group);
    QCOMPARE(loaded.at(0).params.value(QStringLiteral("host")).toString(),
             QStringLiteral("192.168.50.101"));
    QCOMPARE(loaded.at(0).params.value(QStringLiteral("port")).toInt(), 22);
    QCOMPARE(loaded.at(1).group, QStringLiteral("Office"));
}

void DeviceManagerTests::configStoreMissingFile()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());

    ConfigStore store(dir.filePath(QStringLiteral("missing.json")));
    QString error;
    const QList<DeviceConfig> loaded = store.load(&error);

    QVERIFY(loaded.isEmpty());
    QVERIFY(error.isEmpty());
}

void DeviceManagerTests::configStoreMalformedJson()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("broken.json"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ this is not json");
    file.close();

    ConfigStore store(path);
    QString error;
    const QList<DeviceConfig> loaded = store.load(&error);

    QVERIFY(loaded.isEmpty());
    QVERIFY(!error.isEmpty());
}

void DeviceManagerTests::deviceManagerCrud()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("devices.json"));

    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    DeviceManager manager(&plugins, path);
    QSignalSpy addedSpy(&manager, &DeviceManager::deviceAdded);

    DeviceConfig config;
    config.name = QStringLiteral("Lab Device");
    config.pluginId = QStringLiteral("mock");
    config.group = QStringLiteral("Lab");

    QString error;
    const QString id = manager.addDevice(config, &error);
    QVERIFY2(!id.isEmpty(), qPrintable(error));
    QVERIFY(manager.contains(id));
    QCOMPARE(manager.devices().size(), 1);
    QCOMPARE(addedSpy.count(), 1);
    QVERIFY(QFile::exists(path));

    // 从磁盘重新加载
    DeviceManager reloaded(&plugins, path);
    QVERIFY2(reloaded.load(&error), qPrintable(error));
    QCOMPARE(reloaded.devices().size(), 1);
    QCOMPARE(reloaded.device(id).name, QStringLiteral("Lab Device"));
    QCOMPARE(reloaded.device(id).pluginId, QStringLiteral("mock"));

    // 更新
    DeviceConfig updated = reloaded.device(id);
    updated.name = QStringLiteral("Renamed Device");
    QVERIFY2(reloaded.updateDevice(updated, &error), qPrintable(error));
    QCOMPARE(reloaded.device(id).name, QStringLiteral("Renamed Device"));

    // 删除
    QVERIFY2(reloaded.removeDevice(id, &error), qPrintable(error));
    QVERIFY(!reloaded.contains(id));
    QCOMPARE(reloaded.devices().size(), 0);
}

void DeviceManagerTests::deviceManagerRejectsDuplicateId()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("devices.json"));

    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    DeviceManager manager(&plugins, path);

    DeviceConfig config;
    config.id = QStringLiteral("fixed-id");
    config.name = QStringLiteral("Device");
    config.pluginId = QStringLiteral("mock");

    QString error;
    QVERIFY2(!manager.addDevice(config, &error).isEmpty(), qPrintable(error));

    error.clear();
    const QString duplicateId = manager.addDevice(config, &error);
    QVERIFY(duplicateId.isEmpty());
    QVERIFY(error.contains(QStringLiteral("already exists")));
    QCOMPARE(manager.devices().size(), 1);
}

void DeviceManagerTests::deviceManagerCreatesDevice()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("devices.json"));

    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    DeviceManager manager(&plugins, path);

    DeviceConfig config;
    config.name = QStringLiteral("Mock Device");
    config.pluginId = QStringLiteral("mock");

    QString error;
    const QString id = manager.addDevice(config, &error);
    QVERIFY2(!id.isEmpty(), qPrintable(error));

    QScopedPointer<IDevice> device(manager.createDevice(id, nullptr, &error));
    QVERIFY2(device, qPrintable(error));
    QCOMPARE(device->pluginId(), QStringLiteral("mock"));
    QCOMPARE(device->displayName(), QStringLiteral("Mock Device"));

    // 插件不存在时应给出明确错误
    DeviceConfig unknown;
    unknown.name = QStringLiteral("Unknown");
    unknown.pluginId = QStringLiteral("does-not-exist");
    const QString unknownId = manager.addDevice(unknown, &error);
    QVERIFY2(!unknownId.isEmpty(), qPrintable(error));

    error.clear();
    QScopedPointer<IDevice> missing(manager.createDevice(unknownId, nullptr, &error));
    QVERIFY(!missing);
    QVERIFY(error.contains(QStringLiteral("plugin not found")));
}

void DeviceManagerTests::configStoreDashboardLayout()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("devices.json"));

    ConfigStore store(path);
    QString error;

    DeviceConfig device;
    device.id = QStringLiteral("dev-1");
    device.name = QStringLiteral("Device 1");
    device.pluginId = QStringLiteral("mock");
    QVERIFY2(store.save({device}, &error), qPrintable(error));

    DashboardItemLayout cpu;
    cpu.deviceId = QStringLiteral("dev-1");
    cpu.itemId = QStringLiteral("mock_cpu");
    cpu.row = 0;
    cpu.column = 0;
    cpu.columnSpan = 2;

    DashboardItemLayout trend;
    trend.deviceId = QStringLiteral("dev-1");
    trend.itemId = QStringLiteral("mock_trend");
    trend.row = 1;
    trend.column = 0;
    trend.rowSpan = 1;
    trend.columnSpan = 2;
    trend.visible = false;

    QVERIFY2(store.saveDashboardLayout({cpu, trend}, &error), qPrintable(error));

    const QList<DashboardItemLayout> loaded = store.loadDashboardLayout(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loaded.size(), 2);
    QCOMPARE(loaded.at(0).deviceId, QStringLiteral("dev-1"));
    QCOMPARE(loaded.at(0).itemId, QStringLiteral("mock_cpu"));
    QCOMPARE(loaded.at(0).columnSpan, 2);
    QCOMPARE(loaded.at(1).visible, false);

    // 保存布局不应丢掉设备列表
    const QList<DeviceConfig> devices = store.load(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(devices.size(), 1);
    QCOMPARE(devices.first().id, QStringLiteral("dev-1"));
}

void DeviceManagerTests::configStorePluginStatesAndSettings()
{
    QTemporaryDir dir(makeTempDirPath());
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("devices.json"));

    ConfigStore store(path);
    QString error;

    const QHash<QString, bool> states{
        {QStringLiteral("mock"), true},
        {QStringLiteral("ssh"), false},
    };
    QVERIFY2(store.savePluginEnabledStates(states, &error), qPrintable(error));

    AppSettings settings;
    settings.theme = QStringLiteral("dark");
    settings.language = QStringLiteral("en_US");
    QVERIFY2(store.saveAppSettings(settings, &error), qPrintable(error));

    const QHash<QString, bool> loadedStates =
        store.loadPluginEnabledStates(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loadedStates.size(), 2);
    QCOMPARE(loadedStates.value(QStringLiteral("mock"), false), true);
    QCOMPARE(loadedStates.value(QStringLiteral("ssh"), true), false);

    const AppSettings loadedSettings = store.loadAppSettings(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loadedSettings.theme, QStringLiteral("dark"));
    QCOMPARE(loadedSettings.language, QStringLiteral("en_US"));
}

QTEST_GUILESS_MAIN(DeviceManagerTests)

#include "tst_device_manager.moc"
