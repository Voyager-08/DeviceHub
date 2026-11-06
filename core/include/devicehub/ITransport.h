#pragma once

#include "devicehub/Types.h"

#include <QByteArray>
#include <QObject>
#include <QVariantMap>

namespace devicehub {

// 字节流传输层:SSH 通道、串口、TCP 等都实现这个接口
class ITransport : public QObject
{
    Q_OBJECT

public:
    explicit ITransport(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    ~ITransport() override = default;

    // 异步打开,结果通过 stateChanged / errorOccurred 返回
    virtual void open(const QVariantMap &params) = 0;
    virtual void close() = 0;
    virtual void write(const QByteArray &data) = 0;
    virtual ConnectionState state() const = 0;

signals:
    void readyRead(const QByteArray &data);
    void stateChanged(devicehub::ConnectionState state);
    void errorOccurred(const devicehub::ErrorInfo &error);
};

} // namespace devicehub
