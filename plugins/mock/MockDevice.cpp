#include "MockDevice.h"

#include "MockTerminal.h"

#include <QRandomGenerator>
#include <QTimer>

#include <cmath>
#include <utility>

namespace devicehub::test {

MockDevice::MockDevice(QString id, QString displayName, QObject *parent)
    : IDevice(parent)
    , m_id(std::move(id))
    , m_displayName(std::move(displayName))
    , m_transport(new MockTransport(this))
    , m_terminal(new MockTerminal(this))
{
    if (m_displayName.isEmpty())
        m_displayName = m_id;

    // 设备状态直接跟随内部传输层
    connect(m_transport, &ITransport::stateChanged,
            this, &IDevice::stateChanged);
    connect(m_transport, &ITransport::errorOccurred,
            this, &IDevice::errorOccurred);

    // 仪表盘数据每秒刷新一次
    refreshDashboardValues();
    m_dashboardTimer = new QTimer(this);
    m_dashboardTimer->setInterval(1000);
    connect(m_dashboardTimer, &QTimer::timeout,
            this, &MockDevice::refreshDashboardValues);
    m_dashboardTimer->start();
}

MockDevice::~MockDevice() = default;

QString MockDevice::deviceId() const
{
    return m_id;
}

QString MockDevice::displayName() const
{
    return m_displayName;
}

QString MockDevice::pluginId() const
{
    return QStringLiteral("mock");
}

ConnectionState MockDevice::state() const
{
    return m_transport->state();
}

void MockDevice::connectDevice()
{
    m_transport->open(m_connectionParams);
}

void MockDevice::disconnectDevice()
{
    m_transport->close();
}

QStringList MockDevice::capabilityIds() const
{
    return {QStringLiteral("terminal")};
}

ICapability *MockDevice::capability(const QString &id)
{
    if (id == QStringLiteral("terminal"))
        return m_terminal;
    return nullptr;
}

QList<DashboardItemSpec> MockDevice::dashboardItems() const
{
    DashboardItemSpec cpu;
    cpu.id = QStringLiteral("mock_cpu");
    cpu.title = QStringLiteral("CPU 使用率");
    cpu.description = QStringLiteral("模拟的处理器占用百分比");
    cpu.kind = DashboardKind::Gauge;
    cpu.unit = QStringLiteral("%");
    cpu.minValue = 0;
    cpu.maxValue = 100;

    DashboardItemSpec temp;
    temp.id = QStringLiteral("mock_temp");
    temp.title = QStringLiteral("温度");
    temp.description = QStringLiteral("模拟的设备温度");
    temp.kind = DashboardKind::Value;
    temp.unit = QStringLiteral("°C");

    DashboardItemSpec trend;
    trend.id = QStringLiteral("mock_trend");
    trend.title = QStringLiteral("负载趋势");
    trend.description = QStringLiteral("模拟的正弦曲线负载");
    trend.kind = DashboardKind::Trend;
    trend.unit = QStringLiteral("%");
    trend.minValue = 0;
    trend.maxValue = 100;
    trend.defaultSize = QSize(2, 1);

    DashboardItemSpec custom;
    custom.id = QStringLiteral("mock_custom");
    custom.title = QStringLiteral("自定义卡片");
    custom.description = QStringLiteral("由 Mock 插件提供的自定义控件");
    custom.kind = DashboardKind::Custom;

    return {cpu, temp, trend, custom};
}

QVariant MockDevice::dashboardValue(const QString &itemId) const
{
    return m_dashboardValues.value(itemId);
}

void MockDevice::setConnectionParams(const QVariantMap &params)
{
    m_connectionParams = params;
}

void MockDevice::setParams(const QVariantMap &params)
{
    setConnectionParams(params);
    m_transport->open(params);
}

void MockDevice::refreshDashboardValues()
{
    const int cpu = QRandomGenerator::global()->bounded(101);
    const double temperature =
        30.0 + QRandomGenerator::global()->bounded(310) / 10.0;

    m_phase += 0.25;
    const double trend = 50.0 + 40.0 * std::sin(m_phase);

    const QString custom = QStringLiteral("Mock 计数 %1").arg(m_tick++);

    const QVariantMap values{
        {QStringLiteral("mock_cpu"), cpu},
        {QStringLiteral("mock_temp"), temperature},
        {QStringLiteral("mock_trend"), trend},
        {QStringLiteral("mock_custom"), custom},
    };

    m_dashboardValues = values;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        emit dashboardValueChanged(it.key(), it.value());
}

} // namespace devicehub::test
