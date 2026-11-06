#pragma once

#include "devicehub/ITransport.h"

class QTimer;

namespace devicehub::test {

// 测试用传输层:打开即成功,写入的数据原样回显
// 支持的参数:
//   failOnOpen(bool)          —— 打开时故意失败
//   disconnectAfterMs(int)    —— 指定毫秒后模拟断线
//   highFrequency(bool)       —— 每 100ms 主动发一次数据
//   emitIntervalMs(int)       —— 自定义主动发数据的间隔
class MockTransport : public ITransport
{
    Q_OBJECT

public:
    explicit MockTransport(QObject *parent = nullptr);
    ~MockTransport() override = default;

    void open(const QVariantMap &params) override;
    void close() override;
    void write(const QByteArray &data) override;
    ConnectionState state() const override;

    QVariantMap params() const { return m_params; }

    // 测试辅助:模拟设备主动发来数据或发生错误
    void simulateIncoming(const QByteArray &data);
    void simulateError(const ErrorInfo &error);

private:
    void setState(ConnectionState state);
    void stopDataTimer();

    ConnectionState m_state = ConnectionState::Disconnected;
    QVariantMap m_params;
    QTimer *m_dataTimer = nullptr;
};

} // namespace devicehub::test
