#pragma once

#include "devicehub/ICapability.h"

#include <QVariantMap>

namespace devicehub {

// 周期采样/监视能力(CPU、温度、传感器等)
class IMonitor : public ICapability
{
    Q_OBJECT

public:
    explicit IMonitor(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~IMonitor() override = default;

    QString capabilityId() const override { return QStringLiteral("monitor"); }

    virtual void startMonitoring(int intervalMs) = 0;
    virtual void stopMonitoring() = 0;

signals:
    void sampleReceived(const QVariantMap &sample);
};

} // namespace devicehub
