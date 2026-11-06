#include "MockPlugin.h"

#include "devicehub/IDevice.h"
#include "devicehub/Version.h"

#include "MockDevice.h"

#include <QCheckBox>
#include <QLabel>
#include <QPointer>
#include <QVBoxLayout>

namespace devicehub::test {

int MockPlugin::apiVersion() const
{
    return kApiVersion;
}

QStringList MockPlugin::capabilityIds() const
{
    return {QStringLiteral("terminal")};
}

QList<ParamSpec> MockPlugin::paramSpecs() const
{
    ParamSpec host;
    host.key = QStringLiteral("host");
    host.label = QStringLiteral("Host");
    host.type = QStringLiteral("string");
    host.required = true;

    ParamSpec failOnOpen;
    failOnOpen.key = QStringLiteral("failOnOpen");
    failOnOpen.label = QStringLiteral("模拟连接失败");
    failOnOpen.type = QStringLiteral("bool");
    failOnOpen.defaultValue = false;

    return {host, failOnOpen};
}

IDevice *MockPlugin::createDevice(const QVariantMap &params, QObject *parent)
{
    auto *device = new MockDevice(QStringLiteral("mock-1"),
                                  QStringLiteral("Mock Device"),
                                  parent);
    device->setConnectionParams(params);
    return device;
}

QWidget *MockPlugin::createDeviceWidget(IDevice *device, QWidget *parent)
{
    auto *widget = new QWidget(parent);
    auto *layout = new QVBoxLayout(widget);

    auto *title = new QLabel(QStringLiteral("这是 Mock 设备界面"), widget);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    if (device) {
        auto *detail = new QLabel(
            QStringLiteral("设备:%1\n状态:%2")
                .arg(device->displayName(),
                     connectionStateToString(device->state())),
            widget);
        detail->setAlignment(Qt::AlignCenter);
        layout->addWidget(detail);

        QObject::connect(device, &IDevice::stateChanged, detail,
                         [detail, device](ConnectionState) {
                             detail->setText(
                                 QStringLiteral("设备:%1\n状态:%2")
                                     .arg(device->displayName(),
                                          connectionStateToString(device->state())));
                         });
    }

    return widget;
}

QWidget *MockPlugin::createDashboardWidget(const QString &itemId,
                                           IDevice *device,
                                           QWidget *parent)
{
    if (itemId != QStringLiteral("mock_custom") || !device)
        return nullptr;

    auto *label = new QLabel(parent);
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumHeight(60);

    const QPointer<IDevice> guard(device);
    const auto updateText = [label, guard]() {
        if (!guard)
            return;
        label->setText(QStringLiteral("自定义:%1")
                           .arg(guard->dashboardValue(QStringLiteral("mock_custom"))
                                    .toString()));
    };

    updateText();
    QObject::connect(device, &IDevice::dashboardValueChanged, label,
                     [updateText](const QString &changedId, const QVariant &) {
                         if (changedId == QStringLiteral("mock_custom"))
                             updateText();
                     });

    return label;
}

QWidget *MockPlugin::createSettingsWidget(QWidget *parent)
{
    auto *widget = new QWidget(parent);
    auto *layout = new QVBoxLayout(widget);

    auto *title = new QLabel(QStringLiteral("Mock 插件设置"), widget);
    layout->addWidget(title);

    auto *option = new QCheckBox(QStringLiteral("启用模拟高频数据"), widget);
    layout->addWidget(option);

    auto *hint = new QLabel(QStringLiteral("此设置仅用于开发调试。"), widget);
    layout->addWidget(hint);
    layout->addStretch();

    return widget;
}

} // namespace devicehub::test
