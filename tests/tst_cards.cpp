#include "devicehub/Dashboard.h"
#include "ui/common/GaugeCard.h"
#include "ui/common/StatusCard.h"
#include "ui/common/TextCard.h"
#include "ui/common/TrendCard.h"
#include "ui/common/ValueCard.h"

#include <QLabel>
#include <QtTest>

using namespace devicehub;

namespace {

bool hasLabelText(QWidget &widget, const QString &needle)
{
    const auto labels = widget.findChildren<QLabel *>();
    for (QLabel *label : labels) {
        if (label->text().contains(needle))
            return true;
    }
    return false;
}

DashboardItemSpec makeSpec(DashboardKind kind, const QString &unit = {})
{
    DashboardItemSpec spec;
    spec.id = QStringLiteral("test_item");
    spec.title = QStringLiteral("测试项");
    spec.kind = kind;
    spec.unit = unit;
    spec.minValue = 0;
    spec.maxValue = 100;
    return spec;
}

} // namespace

class CardTests : public QObject
{
    Q_OBJECT

private slots:
    void valueCardFormatsNumber();
    void valueCardRejectsText();
    void gaugeAndTrendAcceptValues();
    void statusCardMapsValue();
    void textCardShowsText();
    void offlineStateToggles();
};

void CardTests::valueCardFormatsNumber()
{
    ValueCard card(makeSpec(DashboardKind::Value, QStringLiteral("%")));
    card.setValue(42);
    QVERIFY(hasLabelText(card, QStringLiteral("42")));

    card.setValue(3.5);
    QVERIFY(hasLabelText(card, QStringLiteral("3.5 %")));
}

void CardTests::valueCardRejectsText()
{
    ValueCard card(makeSpec(DashboardKind::Value));
    card.setValue(QStringLiteral("not-a-number"));
    QVERIFY(hasLabelText(card, QStringLiteral("--")));
}

void CardTests::gaugeAndTrendAcceptValues()
{
    GaugeCard gauge(makeSpec(DashboardKind::Gauge, QStringLiteral("%")));
    gauge.setValue(75);
    gauge.setValue(QStringLiteral("bad"));
    QVERIFY(gauge.isOnline());

    TrendCard trend(makeSpec(DashboardKind::Trend));
    trend.setValue(10);
    trend.setValue(20);
    trend.setValue(QStringLiteral("bad"));
    QVERIFY(trend.isOnline());
}

void CardTests::statusCardMapsValue()
{
    StatusCard card(makeSpec(DashboardKind::Status));
    card.setValue(QStringLiteral("warning"));
    QVERIFY(hasLabelText(card, QStringLiteral("警告")));

    card.setValue(true);
    QVERIFY(hasLabelText(card, QStringLiteral("正常")));

    card.setValue(QStringLiteral("unknown-state"));
    QVERIFY(hasLabelText(card, QStringLiteral("--")));
}

void CardTests::textCardShowsText()
{
    TextCard card(makeSpec(DashboardKind::Text));
    card.setValue(QStringLiteral("hello devicehub"));
    QVERIFY(hasLabelText(card, QStringLiteral("hello devicehub")));
}

void CardTests::offlineStateToggles()
{
    ValueCard card(makeSpec(DashboardKind::Value));
    QVERIFY(card.isOnline());
    QVERIFY(card.isEnabled());

    card.setOnline(false);
    QVERIFY(!card.isOnline());
    QVERIFY(!card.isEnabled());

    card.setOnline(true);
    QVERIFY(card.isOnline());
    QVERIFY(card.isEnabled());
}

QTEST_MAIN(CardTests)

#include "tst_cards.moc"
