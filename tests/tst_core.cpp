#include "devicehub/Types.h"
#include "mock/MockDevice.h"
#include "mock/MockTransport.h"

#include <QSignalSpy>
#include <QtTest>

using namespace devicehub;

class CoreTests : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void mockTransportStateSequence();
    void mockTransportEcho();
    void mockTransportErrorWhenClosed();
    void mockDeviceConnectState();
    void stateToString_data();
    void stateToString();
};

void CoreTests::initTestCase()
{
    registerMetaTypes();
}

void CoreTests::mockTransportStateSequence()
{
    test::MockTransport transport;
    QSignalSpy spy(&transport, &ITransport::stateChanged);

    transport.open({{QStringLiteral("host"), QStringLiteral("127.0.0.1")}});

    QCOMPARE(spy.count(), 2);
    QCOMPARE(static_cast<int>(spy.at(0).at(0).value<ConnectionState>()),
             static_cast<int>(ConnectionState::Connecting));
    QCOMPARE(static_cast<int>(spy.at(1).at(0).value<ConnectionState>()),
             static_cast<int>(ConnectionState::Connected));
    QCOMPARE(static_cast<int>(transport.state()),
             static_cast<int>(ConnectionState::Connected));
    QCOMPARE(transport.params().value(QStringLiteral("host")).toString(),
             QStringLiteral("127.0.0.1"));

    transport.close();
    QCOMPARE(static_cast<int>(transport.state()),
             static_cast<int>(ConnectionState::Disconnected));
}

void CoreTests::mockTransportEcho()
{
    test::MockTransport transport;
    transport.open({});

    QSignalSpy spy(&transport, &ITransport::readyRead);
    transport.write(QByteArrayLiteral("hello"));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toByteArray(), QByteArrayLiteral("hello"));
}

void CoreTests::mockTransportErrorWhenClosed()
{
    test::MockTransport transport;
    QSignalSpy spy(&transport, &ITransport::errorOccurred);

    transport.write(QByteArrayLiteral("x"));

    QCOMPARE(spy.count(), 1);
    const auto error = spy.at(0).at(0).value<ErrorInfo>();
    QCOMPARE(static_cast<int>(error.code), static_cast<int>(ErrorCode::NetworkError));
}

void CoreTests::mockDeviceConnectState()
{
    test::MockDevice device(QStringLiteral("dev-1"), QStringLiteral("测试设备"));

    QCOMPARE(device.deviceId(), QStringLiteral("dev-1"));
    QCOMPARE(device.displayName(), QStringLiteral("测试设备"));
    QCOMPARE(device.pluginId(), QStringLiteral("mock"));
    QCOMPARE(static_cast<int>(device.state()),
             static_cast<int>(ConnectionState::Disconnected));

    QSignalSpy spy(&device, &IDevice::stateChanged);
    device.connectDevice();

    QCOMPARE(static_cast<int>(device.state()),
             static_cast<int>(ConnectionState::Connected));
    QCOMPARE(spy.count(), 2);

    device.disconnectDevice();
    QCOMPARE(static_cast<int>(device.state()),
             static_cast<int>(ConnectionState::Disconnected));
}

void CoreTests::stateToString_data()
{
    QTest::addColumn<int>("state");
    QTest::addColumn<QString>("text");

    QTest::newRow("Disconnected") << static_cast<int>(ConnectionState::Disconnected)
                                  << QStringLiteral("Disconnected");
    QTest::newRow("Connecting") << static_cast<int>(ConnectionState::Connecting)
                                << QStringLiteral("Connecting");
    QTest::newRow("Connected") << static_cast<int>(ConnectionState::Connected)
                               << QStringLiteral("Connected");
    QTest::newRow("Reconnecting") << static_cast<int>(ConnectionState::Reconnecting)
                                  << QStringLiteral("Reconnecting");
    QTest::newRow("Error") << static_cast<int>(ConnectionState::Error)
                           << QStringLiteral("Error");
}

void CoreTests::stateToString()
{
    QFETCH(int, state);
    QFETCH(QString, text);

    QCOMPARE(connectionStateToString(static_cast<ConnectionState>(state)), text);
}

QTEST_GUILESS_MAIN(CoreTests)

#include "tst_core.moc"
