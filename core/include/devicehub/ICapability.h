#pragma once

#include <QObject>
#include <QString>

namespace devicehub {

// 能力基类:UI 只关心设备是否具备某能力,不关心设备类型
class ICapability : public QObject
{
    Q_OBJECT

public:
    explicit ICapability(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~ICapability() override = default;

    virtual QString capabilityId() const = 0;
};

} // namespace devicehub
