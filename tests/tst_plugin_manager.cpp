#include "devicehub/IDevice.h"
#include "devicehub/IPlugin.h"
#include "devicehub/Types.h"
#include "devicehub/Version.h"
#include "manager/PluginManager.h"

#include <QScopedPointer>
#include <QtTest>

using namespace devicehub;

class PluginManagerTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void loadsValidPlugin();
    void rejectsWrongApiVersion();
    void reportsMissingDirectory();
    void disabledPluginIsRecorded();
};

void PluginManagerTests::initTestCase()
{
    registerMetaTypes();
}

void PluginManagerTests::loadsValidPlugin()
{
    PluginManager manager;
    manager.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));

    QCOMPARE(manager.loadAll(), 1);
    QVERIFY(manager.pluginIds().contains(QStringLiteral("mock")));
    QVERIFY(!manager.pluginIds().contains(QStringLiteral("mock_bad")));

    IPlugin *plugin = manager.plugin(QStringLiteral("mock"));
    QVERIFY(plugin);
    QCOMPARE(plugin->apiVersion(), kApiVersion);
    QCOMPARE(plugin->displayName(), QStringLiteral("Mock Plugin"));
    QCOMPARE(plugin->version(), QStringLiteral("1.0.0"));
    QCOMPARE(plugin->paramSpecs().size(), 2);

    const PluginInfo info = manager.pluginInfo(QStringLiteral("mock"));
    QCOMPARE(static_cast<int>(info.state), static_cast<int>(PluginState::Loaded));
    QCOMPARE(info.version, QStringLiteral("1.0.0"));
    QCOMPARE(info.apiVersion, kApiVersion);
    QVERIFY(info.capabilityIds.contains(QStringLiteral("terminal")));

    QScopedPointer<IDevice> device(plugin->createDevice({}, nullptr));
    QVERIFY(device);
    QCOMPARE(device->pluginId(), QStringLiteral("mock"));
}

void PluginManagerTests::rejectsWrongApiVersion()
{
    PluginManager manager;
    manager.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));

    QCOMPARE(manager.loadAll(), 1);
    QVERIFY(!manager.plugin(QStringLiteral("mock_bad")));

    const auto errors = manager.errors();
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.first().pluginId, QStringLiteral("mock_bad"));
    QVERIFY(errors.first().message.contains(QStringLiteral("API version")));
}

void PluginManagerTests::reportsMissingDirectory()
{
    PluginManager manager;
    manager.setPluginDirectory(QStringLiteral("devicehub-missing-plugin-dir"));

    QCOMPARE(manager.loadAll(), 0);
    QCOMPARE(manager.errors().size(), 1);
}

void PluginManagerTests::disabledPluginIsRecorded()
{
    PluginManager manager;
    manager.setPluginEnabled(QStringLiteral("mock"), false);
    manager.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));

    QCOMPARE(manager.loadAll(), 0);
    QVERIFY(!manager.plugin(QStringLiteral("mock")));
    QCOMPARE(manager.disabledPluginIds(),
             QStringList{QStringLiteral("mock")});

    const PluginInfo info = manager.pluginInfo(QStringLiteral("mock"));
    QCOMPARE(static_cast<int>(info.state), static_cast<int>(PluginState::Disabled));
    QCOMPARE(info.version, QStringLiteral("1.0.0"));
    QCOMPARE(info.apiVersion, kApiVersion);
    QVERIFY(info.capabilityIds.contains(QStringLiteral("terminal")));

    // 版本不兼容的插件依然按失败记录
    QCOMPARE(manager.failedPluginInfos().size(), 1);
    QCOMPARE(manager.failedPluginInfos().first().pluginId,
             QStringLiteral("mock_bad"));
}

QTEST_GUILESS_MAIN(PluginManagerTests)

#include "tst_plugin_manager.moc"
