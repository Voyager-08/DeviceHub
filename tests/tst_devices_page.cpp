#include "devicehub/Types.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "manager/SessionManager.h"
#include "ui/pages/DevicesPage.h"
#include "ui/pages/NewDeviceDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTemporaryDir>
#include <QTreeWidget>
#include <QtTest>

using namespace devicehub;

class DevicesPageTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void showsMockDeviceWidget();
    void generatesFormFromParams();
};

void DevicesPageTests::initTestCase()
{
    registerMetaTypes();
}

void DevicesPageTests::showsMockDeviceWidget()
{
    const QString base = QStringLiteral(DEVICEHUB_TEST_TEMP_DIR);
    QDir().mkpath(base);
    QTemporaryDir dir(base + QStringLiteral("/devices-XXXXXX"));
    QVERIFY(dir.isValid());

    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    DeviceManager manager(&plugins, dir.filePath(QStringLiteral("devices.json")));

    DeviceConfig config;
    config.name = QStringLiteral("Mock 设备");
    config.pluginId = QStringLiteral("mock");

    QString error;
    const QString id = manager.addDevice(config, &error);
    QVERIFY2(!id.isEmpty(), qPrintable(error));

    SessionManager session(&manager);
    DevicesPage page(&plugins, &manager, &session);
    page.show();
    QTest::qWait(50);

    auto *tree = page.findChild<QTreeWidget *>();
    QVERIFY(tree);
    QCOMPARE(tree->topLevelItemCount(), 1);
    QCOMPARE(tree->topLevelItem(0)->childCount(), 1);
    QCOMPARE(tree->topLevelItem(0)->child(0)->text(1), QStringLiteral("未连接"));

    page.openDevice(id);
    QTest::qWait(50);

    QCOMPARE(tree->topLevelItem(0)->child(0)->text(1), QStringLiteral("已连接"));

    auto *stack = page.findChild<QStackedWidget *>();
    QVERIFY(stack);
    QWidget *current = stack->currentWidget();
    QVERIFY(current);

    bool foundTitle = false;
    const auto labels = current->findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text().contains(QStringLiteral("这是 Mock 设备界面"))) {
            foundTitle = true;
            break;
        }
    }
    QVERIFY(foundTitle);
}

void DevicesPageTests::generatesFormFromParams()
{
    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    NewDeviceDialog dialog(&plugins);

    auto *pluginCombo = dialog.findChild<QComboBox *>();
    QVERIFY(pluginCombo);
    QCOMPARE(pluginCombo->currentData().toString(), QStringLiteral("mock"));

    // 名称、分组 + host 参数
    QVERIFY(dialog.findChildren<QLineEdit *>().size() >= 3);
    // failOnOpen 参数应生成复选框
    QVERIFY(dialog.findChild<QCheckBox *>() != nullptr);
}

QTEST_MAIN(DevicesPageTests)

#include "tst_devices_page.moc"
