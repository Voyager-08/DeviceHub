#include "MockTransport.h"

#include <QTimer>

namespace devicehub::test {

MockTransport::MockTransport(QObject *parent)
    : ITransport(parent)
{
}

void MockTransport::open(const QVariantMap &params)
{
    m_params = params;
    stopDataTimer();

    setState(ConnectionState::Connecting);

    // 故意失败:用于测试错误路径
    if (params.value(QStringLiteral("failOnOpen")).toBool()) {
        setState(ConnectionState::Error);
        emit errorOccurred(ErrorInfo{ErrorCode::NetworkError,
                                     QStringLiteral("mock connection failure")});
        return;
    }

    setState(ConnectionState::Connected);

    // 定时断线
    const int disconnectAfterMs =
        params.value(QStringLiteral("disconnectAfterMs")).toInt();
    if (disconnectAfterMs > 0) {
        QTimer::singleShot(disconnectAfterMs, this, [this]() {
            if (m_state == ConnectionState::Connected
                || m_state == ConnectionState::Reconnecting) {
                simulateError(ErrorInfo{ErrorCode::Timeout,
                                        QStringLiteral("mock connection lost")});
            }
        });
    }

    // 主动发数据,模拟高频数据源
    int intervalMs = params.value(QStringLiteral("emitIntervalMs")).toInt();
    if (params.value(QStringLiteral("highFrequency")).toBool())
        intervalMs = 100;

    if (intervalMs > 0) {
        m_dataTimer = new QTimer(this);
        connect(m_dataTimer, &QTimer::timeout, this, [this]() {
            if (m_state == ConnectionState::Connected)
                emit readyRead(QByteArrayLiteral("mock-data\n"));
        });
        m_dataTimer->start(intervalMs);
    }
}

void MockTransport::close()
{
    stopDataTimer();

    if (m_state == ConnectionState::Disconnected)
        return;
    setState(ConnectionState::Disconnected);
}

void MockTransport::write(const QByteArray &data)
{
    if (m_state != ConnectionState::Connected) {
        emit errorOccurred(ErrorInfo{ErrorCode::NetworkError,
                                     QStringLiteral("transport is not connected")});
        return;
    }

    // 模拟回显
    emit readyRead(data);
}

ConnectionState MockTransport::state() const
{
    return m_state;
}

void MockTransport::simulateIncoming(const QByteArray &data)
{
    emit readyRead(data);
}

void MockTransport::simulateError(const ErrorInfo &error)
{
    stopDataTimer();
    setState(ConnectionState::Error);
    emit errorOccurred(error);
}

void MockTransport::setState(ConnectionState state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged(m_state);
}

void MockTransport::stopDataTimer()
{
    if (!m_dataTimer)
        return;
    m_dataTimer->stop();
    m_dataTimer->deleteLater();
    m_dataTimer = nullptr;
}

} // namespace devicehub::test
