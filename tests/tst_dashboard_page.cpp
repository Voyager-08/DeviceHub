#include "devicehub/Types.h"
#include "manager/DeviceManager.h"
#include "manager/PluginManager.h"
#include "manager/SessionManager.h"
#include "storage/ConfigStore.h"
#include "ui/common/DashboardCard.h"
#include "ui/common/ValueCard.h"
#include "ui/pages/DashboardPage.h"

#include <QDir>
#include <QLabel>
#include <QTemporaryDir>
#include <QtTest>

using namespace devicehub;

namespace {

bool hasLabelContaining(QWidget &widget, const QString &needle)
{
    const auto labels = widget.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text().contains(needle))
            return true;
    }
    return false;
}

} // namespace

class DashboardPageTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void buildsCardsForMockDevice();
};

void DashboardPageTests::initTestCase()
{
    registerMetaTypes();
}

void DashboardPageTests::buildsCardsForMockDevice()
{
    const QString base = QStringLiteral(DEVICEHUB_TEST_TEMP_DIR);
    QDir().mkpath(base);
    QTemporaryDir dir(base + QStringLiteral("/dashboard-XXXXXX"));
    QVERIFY(dir.isValid());

    PluginManager plugins;
    plugins.setPluginDirectory(QStringLiteral(DEVICEHUB_TEST_PLUGIN_DIR));
    QCOMPARE(plugins.loadAll(), 1);

    DeviceManager manager(&plugins, dir.filePath(QStringLiteral("devices.json")));

    DeviceConfig config;
    config.name = QStringLiteral("仪表盘设备");
    config.pluginId = QStringLiteral("mock");

    QString error;
    const QString id = manager.addDevice(config, &error);
    QVERIFY2(!id.isEmpty(), qPrintable(error));

    SessionManager session(&manager);
    DashboardPage page(&plugins, &manager, &session);
    page.show();

    session.openDevice(id);
    QTest::qWait(100);

    const auto cards = page.findChildren<DashboardCard *>();
    QCOMPARE(cards.size(), 4);
    for (DashboardCard *card : cards)
        QVERIFY(card->isOnline());

    // 数值卡应该有真实值(单位 °C),而不是占位 "--"
    const auto valueCards = page.findChildren<ValueCard *>();
    QCOMPARE(valueCards.size(), 1);
    QVERIFY(hasLabelContaining(*valueCards.first(), QStringLiteral("°C")));

    // Custom 卡片内容来自插件
    QVERIFY(hasLabelContaining(page, QStringLiteral("自定义")));

    // 布局应已保存:4 个设备项,位置有效且可见
    const QList<DashboardItemLayout> layout =
        manager.configStore().loadDashboardLayout(&error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(layout.size(), 4);
    for (const DashboardItemLayout &item : layout) {
        QCOMPARE(item.deviceId, id);
        QVERIFY(item.visible);
        QVERIFY(item.row >= 0);
        QVERIFY(item.column >= 0);
    }

    // 断开后卡片变灰并保留
    session.disconnectDevice(id);
    QTest::qWait(50);

    const auto offlineCards = page.findChildren<DashboardCard *>();
    QCOMPARE(offlineCards.size(), 4);
    for (DashboardCard *card : offlineCards) {
        QVERIFY(!card->isOnline());
        QVERIFY(!card->isEnabled());
    }
}

QTEST_MAIN(DashboardPageTests)

#include "tst_dashboard_page.moc"
