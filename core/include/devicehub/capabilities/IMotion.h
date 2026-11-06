#pragma once

#include "devicehub/ICapability.h"

#include <QVariantMap>

namespace devicehub {

// 运动控制能力(机械臂、云台、滑台等)
class IMotion : public ICapability
{
    Q_OBJECT

public:
    explicit IMotion(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~IMotion() override = default;

    QString capabilityId() const override { return QStringLiteral("motion"); }

    virtual void moveTo(const QVariantMap &target) = 0;
    virtual void stopMotion() = 0;
    virtual bool isMoving() const = 0;

signals:
    void positionChanged(const QVariantMap &position);
    void motionFinished(bool ok);
};

} // namespace devicehub
