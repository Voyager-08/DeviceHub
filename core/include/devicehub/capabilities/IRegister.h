#pragma once

#include "devicehub/ICapability.h"

#include <QVariant>

namespace devicehub {

// 寄存器读写能力(Modbus、PLC、板卡等)
class IRegister : public ICapability
{
    Q_OBJECT

public:
    explicit IRegister(QObject *parent = nullptr)
        : ICapability(parent)
    {
    }

    ~IRegister() override = default;

    QString capabilityId() const override { return QStringLiteral("register"); }

    virtual int registerCount() const = 0;
    virtual void readRegister(int index) = 0;
    virtual void writeRegister(int index, const QVariant &value) = 0;

signals:
    void registerRead(int index, const QVariant &value);
    void registerWritten(int index, bool ok);
};

} // namespace devicehub
